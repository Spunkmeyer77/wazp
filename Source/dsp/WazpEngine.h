#pragma once
// WAZP sound engine - a monophonic late-70s style synth voice.
// Translated from the WOSP Max for Live device (Ticco Ross):
//   2 oscillators (32'-2', OFF / pulse / saw, pulse width, osc 2 pitch, hard sync)
//   mixer (osc 1, osc 2, noise) -> soft-saturating state-variable filter (LO / BAND / HI)
//   control oscillator (sine / square / noise / random) -> pitch and filter
//   VCA envelope (attack, decay, repeat / sustain level) and control envelope (attack, decay, repeat / delay)
#include <cstdint>
#include <cmath>
#include <array>
#include <algorithm>

namespace wazp
{

struct Params
{
    float glide = 0.0f;          // 0..1
    bool  sync = false;
    int   osc1Range = 2;         // 0..4 = 32' 16' 8' 4' 2'
    float osc1Width = 50.0f;     // 10..90 %
    int   osc1Wave = 2;          // 0 off, 1 pulse, 2 saw
    int   osc2Range = 2;
    float osc2Pitch = 0.0f;      // semitones -12..12
    int   osc2Wave = 2;
    float lfoRate = 2.0f;        // Hz
    float lfoPitch = 0.0f;       // 0..1
    float noise = 0.0f;          // 0..1
    int   lfoWave = 0;           // 0 sine, 1 square, 2 noise, 3 random
    float cutoff = 8000.0f;      // Hz
    float resonance = 0.2f;      // 0..1
    int   filterMode = 0;        // 0 LO, 1 BAND, 2 HI
    float filterEnv = 0.0f;      // -1..1
    float filterLfo = 0.0f;      // -1..1
    float vcaAttack = 0.0f, vcaDecay = 0.4f, vcaSustain = 0.7f;   // 0..1
    float envAttack = 0.0f, envDecay = 0.4f, envDelay = 0.0f;     // 0..1
    float osc1Level = 0.8f, osc2Level = 0.8f;                     // 0..1
    float volumeDb = -6.0f;
};

// Attack / decay / (repeat | sustain-or-delay) envelope.
struct Envelope
{
    enum Stage { Idle, Delay, Attack, Decay, Sustain, Release };
    Stage stage = Idle;
    double value = 0.0, sr = 44100.0, delayLeft = 0.0;

    void reset() { stage = Idle; value = 0.0; }
    void trigger(double delaySeconds)
    {
        if (delaySeconds > 0.0005) { stage = Delay; delayLeft = delaySeconds * sr; }
        else stage = Attack;
    }
    void release() { if (stage != Idle) stage = Release; }

    // attackS/decayS in seconds; sustain 0..1; repeat = loop attack/decay while held
    float next(double attackS, double decayS, double sustain, bool repeat)
    {
        switch (stage)
        {
            case Idle: value = 0.0; break;
            case Delay: if (--delayLeft <= 0.0) stage = Attack; break;
            case Attack:
                value += 1.0 / std::max(1.0, attackS * sr);
                if (value >= 1.0) { value = 1.0; stage = Decay; }
                break;
            case Decay:
            {
                double target = repeat ? 0.0 : sustain;
                double coef = std::exp(-4.6 / std::max(1.0, decayS * sr));
                value = target + (value - target) * coef;
                if (repeat && value < 0.002) { value = 0.0; stage = Attack; }
                else if (!repeat && std::abs(value - target) < 0.0005) stage = Sustain;
                break;
            }
            case Sustain: if (repeat) stage = Attack; else value = sustain; break;
            case Release:
            {
                double coef = std::exp(-4.6 / std::max(1.0, decayS * sr));
                value *= coef;
                if (value < 0.00005) { value = 0.0; stage = Idle; }
                break;
            }
        }
        return (float) value;
    }
    bool active() const { return stage != Idle; }
};

class Engine
{
public:
    void prepare(double sampleRate);
    void reset();
    void noteOn(int note);
    void noteOff(int note);
    void allNotesOff();
    void setPitchBend(float semis) { bend = semis; }
    void setModWheel(float v) { modWheel = v; }
    // renders mono into out[0..n)
    void render(float* out, int n, const Params& p);
    bool isActive() const { return vcaEnv.active(); }

private:
    float polyBlep(double t, double dt) const;
    float nextNoise();

    double sr = 44100.0;
    std::array<int, 16> stack {};
    int stackSize = 0;
    int currentNote = -1;
    double targetPitch = 60.0, glidePitch = 60.0;
    double phase1 = 0.0, phase2 = 0.0;
    double lfoPhase = 0.0;
    float lfoHeld = 0.0f, lfoNoiseSmooth = 0.0f;
    float bend = 0.0f, modWheel = 0.0f;
    float ic1 = 0.0f, ic2 = 0.0f;       // SVF state
    float smoothCutoff = 8000.0f, smoothGain = 0.5f, dcX = 0.0f, dcY = 0.0f;
    uint32_t rng = 0x12345678u;
    bool pendingTrigger = false, pendingRelease = false;
    Envelope vcaEnv, ctrlEnv;
};

} // namespace wazp
