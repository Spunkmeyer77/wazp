#include "WazpEngine.h"
#include <algorithm>

namespace wazp
{

static constexpr double kTwoPi = 6.283185307179586;
static const double kRange[5] = { 0.25, 0.5, 1.0, 2.0, 4.0 };   // 32' 16' 8' 4' 2'

// knob 0..1 -> seconds, exponential (1 ms .. maxS)
static double knobTime(float v, double minS, double maxS)
{
    return minS * std::pow(maxS / minS, (double) std::clamp(v, 0.0f, 1.0f));
}

// the repeat zone at the bottom of the third envelope knob
static constexpr float kRepeatZone = 0.04f;

void Engine::prepare(double sampleRate)
{
    sr = sampleRate;
    vcaEnv.sr = ctrlEnv.sr = sampleRate;
    reset();
}

void Engine::reset()
{
    stackSize = 0;
    currentNote = -1;
    phase1 = phase2 = lfoPhase = 0.0;
    ic1 = ic2 = 0.0f;
    dcX = dcY = 0.0f;
    vcaEnv.reset();
    ctrlEnv.reset();
}

void Engine::noteOn(int note)
{
    // remove if already held, then push (last-note priority)
    for (int i = 0; i < stackSize; ++i)
        if (stack[(size_t) i] == note) { std::copy(stack.begin() + i + 1, stack.begin() + stackSize, stack.begin() + i); --stackSize; break; }
    if (stackSize < (int) stack.size()) stack[(size_t) stackSize++] = note;

    if (currentNote < 0 && !vcaEnv.active())
        glidePitch = note;                     // first note after silence: no glide from an old pitch
    currentNote = note;
    targetPitch = note;
    pendingTrigger = true;
}

void Engine::noteOff(int note)
{
    for (int i = 0; i < stackSize; ++i)
        if (stack[(size_t) i] == note) { std::copy(stack.begin() + i + 1, stack.begin() + stackSize, stack.begin() + i); --stackSize; break; }

    if (stackSize > 0)
    {
        int prev = stack[(size_t) stackSize - 1];
        if (prev != currentNote) { currentNote = prev; targetPitch = prev; }   // fall back legato
    }
    else
    {
        currentNote = -1;
        pendingRelease = true;
    }
}

void Engine::allNotesOff()
{
    stackSize = 0;
    currentNote = -1;
    vcaEnv.release();
    ctrlEnv.release();
}

float Engine::polyBlep(double t, double dt) const
{
    if (t < dt) { t /= dt; return (float) (t + t - t * t - 1.0); }
    if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
    return 0.0f;
}

float Engine::nextNoise()
{
    rng = rng * 1664525u + 1013904223u;
    return (float) ((int32_t) rng) * (1.0f / 2147483648.0f);
}

void Engine::render(float* out, int n, const Params& p)
{
    const bool vcaRepeat = p.vcaSustain <= kRepeatZone;
    const double vcaSus = vcaRepeat ? 0.0 : (p.vcaSustain - kRepeatZone) / (1.0f - kRepeatZone);
    const bool envRepeat = p.envDelay <= kRepeatZone;
    const double envDelayS = envRepeat ? 0.0 : knobTime((p.envDelay - kRepeatZone) / (1.0f - kRepeatZone), 0.002, 2.0);

    const double vA = knobTime(p.vcaAttack, 0.001, 2.0), vD = knobTime(p.vcaDecay, 0.01, 4.0);
    const double eA = knobTime(p.envAttack, 0.001, 2.0), eD = knobTime(p.envDecay, 0.01, 4.0);

    const double glideS = p.glide <= 0.001f ? 0.0 : knobTime(p.glide, 0.005, 2.0);
    const double glideCoef = glideS <= 0.0 ? 0.0 : std::exp(-1.0 / (glideS * 0.33 * sr));

    const float gain1 = p.osc1Wave == 0 ? 0.0f : p.osc1Level * p.osc1Level;
    const float gain2 = p.osc2Wave == 0 ? 0.0f : p.osc2Level * p.osc2Level;
    const float gainN = p.noise * p.noise;
    const float width = std::clamp(p.osc1Width, 10.0f, 90.0f) * 0.01f;
    const float volTarget = p.volumeDb <= -79.5f ? 0.0f : std::pow(10.0f, p.volumeDb / 20.0f);

    // pitch modulation depth: up to +-12 semitones, mod wheel adds up to half that
    const float pitchDepth = 12.0f * (p.lfoPitch * p.lfoPitch) + 6.0f * modWheel;
    // filter mod amounts: exponential feel like the WOSP expr (sign * (e^3|x| - 1)/(e^3 - 1))
    auto expAmt = [](float x) { float a = std::abs(x); float c = (std::exp(a * 3.0f) - 1.0f) / (std::exp(3.0f) - 1.0f); return x < 0 ? -c : c; };
    const float envOct = 7.0f * expAmt(p.filterEnv);
    const float lfoOct = 4.0f * expAmt(p.filterLfo);
    const float k = 2.0f - 1.97f * std::pow(std::clamp(p.resonance, 0.0f, 1.0f), 0.8f);

    if (pendingTrigger)
    {
        vcaEnv.trigger(0.0);
        ctrlEnv.trigger(envDelayS);
        pendingTrigger = false;
    }
    if (pendingRelease) { vcaEnv.release(); ctrlEnv.release(); pendingRelease = false; }

    for (int i = 0; i < n; ++i)
    {
        // ---- control oscillator ------------------------------------------------------
        lfoPhase += p.lfoRate / sr;
        bool wrapped = false;
        if (lfoPhase >= 1.0) { lfoPhase -= 1.0; wrapped = true; }
        float lfo = 0.0f;
        switch (p.lfoWave)
        {
            case 0: lfo = (float) std::sin(kTwoPi * lfoPhase); break;
            case 1: lfo = lfoPhase < 0.5 ? 1.0f : -1.0f; break;
            case 2: lfoNoiseSmooth += 0.02f * (nextNoise() - lfoNoiseSmooth); lfo = std::clamp(lfoNoiseSmooth * 4.0f, -1.0f, 1.0f); break;
            default: if (wrapped) lfoHeld = nextNoise(); lfo = lfoHeld; break;
        }

        // ---- envelopes ---------------------------------------------------------------
        const float vca = vcaEnv.next(vA, vD, vcaSus, vcaRepeat);
        const float cenv = ctrlEnv.next(eA, eD, 0.0, envRepeat);

        // ---- pitch -------------------------------------------------------------------
        glidePitch = glideCoef <= 0.0 ? targetPitch : targetPitch + (glidePitch - targetPitch) * glideCoef;
        const double basePitch = glidePitch + bend + pitchDepth * lfo;
        const double f1 = 440.0 * std::pow(2.0, (basePitch - 69.0) / 12.0) * kRange[std::clamp(p.osc1Range, 0, 4)];
        const double f2 = 440.0 * std::pow(2.0, (basePitch + p.osc2Pitch - 69.0) / 12.0) * kRange[std::clamp(p.osc2Range, 0, 4)];
        const double dt1 = std::min(0.45, f1 / sr), dt2 = std::min(0.45, f2 / sr);

        // ---- oscillators -------------------------------------------------------------
        phase1 += dt1;
        bool reset1 = false;
        if (phase1 >= 1.0) { phase1 -= 1.0; reset1 = true; }
        phase2 += dt2;
        if (p.sync && reset1) phase2 = phase1 * (dt2 / dt1);   // hard sync osc 2 to osc 1
        if (phase2 >= 1.0) phase2 -= std::floor(phase2);

        float o1 = 0.0f, o2 = 0.0f;
        if (p.osc1Wave == 2) o1 = (float) (2.0 * phase1 - 1.0) - polyBlep(phase1, dt1);
        else if (p.osc1Wave == 1)
        {
            double t2 = phase1 + (1.0 - width); if (t2 >= 1.0) t2 -= 1.0;
            o1 = (phase1 < width ? 1.0f : -1.0f) + polyBlep(phase1, dt1) - polyBlep(t2, dt1);
        }
        if (p.osc2Wave == 2) o2 = (float) (2.0 * phase2 - 1.0) - polyBlep(phase2, dt2);
        else if (p.osc2Wave == 1)
        {
            double t2 = phase2 + 0.5; if (t2 >= 1.0) t2 -= 1.0;
            o2 = (phase2 < 0.5 ? 1.0f : -1.0f) + polyBlep(phase2, dt2) - polyBlep(t2, dt2);
        }

        float mix = 0.5f * (o1 * gain1 + o2 * gain2) + 0.6f * nextNoise() * gainN;

        // ---- filter (TPT state variable, saturating) ----------------------------------
        float cutoff = p.cutoff * std::exp2(envOct * cenv + lfoOct * lfo);
        smoothCutoff += 0.01f * (cutoff - smoothCutoff);
        const float fc = std::clamp(smoothCutoff, 20.0f, (float) (sr * 0.45));
        const float g = std::tan(3.14159265f * fc / (float) sr);
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float in = std::tanh(mix * 1.3f);
        const float v3 = in - ic2;
        const float v1 = a1 * ic1 + g * a1 * v3;
        const float v2 = ic2 + g * v1;
        ic1 = std::tanh(2.0f * v1 - ic1);
        ic2 = 2.0f * v2 - ic2;
        float y = p.filterMode == 0 ? v2 : p.filterMode == 1 ? v1 * k : in - k * v1 - v2;

        // ---- VCA + output ------------------------------------------------------------
        smoothGain += 0.002f * (volTarget - smoothGain);
        y = std::tanh(y * vca * 1.2f) * smoothGain;
        // DC blocker
        const float dcOut = y - dcX + 0.995f * dcY;
        dcX = y; dcY = dcOut;
        out[i] = dcOut;
    }
}

} // namespace wazp
