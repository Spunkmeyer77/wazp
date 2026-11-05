#pragma once
// Factory presets and the user preset folder.
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wazp
{

struct FactoryPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;   // parameter id -> real value (unlisted = default)
};

inline const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> p = {
        { "INIT", {} },
        { "BUZZ BASS", { {"osc1Range",1}, {"osc2Range",1}, {"osc2Pitch",0.08f}, {"cutoff",420}, {"resonance",0.45f}, {"filterEnv",0.55f},
                         {"envDecay",0.35f}, {"vcaDecay",0.35f}, {"vcaSustain",0.55f} } },
        { "WASP STING", { {"osc1Wave",1}, {"osc1Width",22}, {"osc2Pitch",7}, {"cutoff",900}, {"resonance",0.7f}, {"filterEnv",0.7f},
                          {"envDecay",0.25f}, {"vcaDecay",0.3f}, {"vcaSustain",0.3f} } },
        { "SYNC SCREAM", { {"sync",1}, {"osc2Range",3}, {"osc2Pitch",5}, {"osc1Level",0.3f}, {"cutoff",2500}, {"resonance",0.5f},
                           {"filterEnv",0.4f}, {"lfoPitch",0.0f}, {"glide",0.25f} } },
        { "REPEAT PERC", { {"osc1Wave",1}, {"cutoff",1200}, {"resonance",0.8f}, {"filterEnv",0.6f}, {"vcaSustain",0.0f}, {"vcaAttack",0.0f},
                           {"vcaDecay",0.2f}, {"envDelay",0.0f}, {"envDecay",0.2f} } },
        { "DREXY ACID", { {"osc2Wave",0}, {"osc1Range",1}, {"cutoff",300}, {"resonance",0.88f}, {"filterEnv",0.75f}, {"envDecay",0.3f},
                          {"vcaDecay",0.45f}, {"vcaSustain",0.6f}, {"glide",0.2f} } },
        { "NOISE WIND", { {"osc1Wave",0}, {"osc2Wave",0}, {"noise",0.9f}, {"filterMode",1}, {"cutoff",1200}, {"resonance",0.85f},
                          {"filterLfo",0.5f}, {"lfoRate",0.3f}, {"vcaAttack",0.6f}, {"vcaDecay",0.7f}, {"vcaSustain",1.0f} } },
        { "SIREN", { {"lfoPitch",0.45f}, {"lfoRate",1.5f}, {"lfoWave",0}, {"cutoff",5000}, {"vcaSustain",1.0f} } },
        { "RANDOM BLEEPS", { {"lfoWave",3}, {"lfoRate",8}, {"lfoPitch",0.6f}, {"osc2Wave",0}, {"osc1Wave",1}, {"cutoff",3000},
                             {"resonance",0.6f}, {"filterLfo",0.4f}, {"vcaSustain",1.0f} } },
        { "HOLLOW LEAD", { {"osc1Wave",1}, {"osc1Width",50}, {"osc2Wave",1}, {"osc2Range",3}, {"osc2Pitch",-0.1f}, {"cutoff",2200},
                           {"resonance",0.3f}, {"lfoPitch",0.08f}, {"lfoRate",5.5f}, {"glide",0.15f}, {"vcaSustain",0.9f} } },
        { "HIGH PASS FIZZ", { {"filterMode",2}, {"cutoff",900}, {"resonance",0.5f}, {"osc2Pitch",12}, {"noise",0.3f}, {"vcaSustain",0.8f} } },
        { "BAND PLUCK", { {"filterMode",1}, {"cutoff",700}, {"resonance",0.75f}, {"filterEnv",0.8f}, {"envDecay",0.25f},
                          {"vcaDecay",0.3f}, {"vcaSustain",0.15f} } },
        { "SUB DRONE", { {"osc1Range",0}, {"osc2Range",1}, {"osc2Pitch",0.12f}, {"cutoff",600}, {"filterLfo",0.25f}, {"lfoRate",0.15f},
                         {"vcaAttack",0.5f}, {"vcaSustain",1.0f} } },
        { "WOBBLE", { {"osc1Range",1}, {"cutoff",500}, {"resonance",0.6f}, {"filterLfo",0.6f}, {"lfoWave",1}, {"lfoRate",4}, {"vcaSustain",1.0f} } },
        { "SLOW SWELL", { {"vcaAttack",0.8f}, {"envAttack",0.8f}, {"filterEnv",0.5f}, {"cutoff",400}, {"vcaSustain",0.9f}, {"envDelay",0.5f} } },
        { "BEATING DETUNE", { {"pitchDest",2}, {"lfoPitch",0.12f}, {"lfoRate",0.6f}, {"cutoff",1800}, {"resonance",0.35f},
                              {"vcaSustain",0.9f}, {"filterEnv",0.3f} } },
        { "SYNC SWEEP", { {"sync",1}, {"pitchDest",2}, {"osc2Range",3}, {"lfoPitch",0.55f}, {"lfoRate",0.4f}, {"osc1Level",0.35f},
                          {"cutoff",4000}, {"resonance",0.4f}, {"vcaSustain",0.9f} } },
        { "NOTCH PHASER", { {"filterMode",3}, {"cutoff",900}, {"resonance",0.75f}, {"filterLfo",0.3f}, {"lfoRate",0.35f},
                            {"osc2Pitch",0.1f}, {"vcaSustain",1.0f} } },
        { "8-BIT SQUARE", { {"osc1Wave",1}, {"osc1Width",12}, {"osc2Wave",0}, {"cutoff",20000}, {"resonance",0.0f}, {"vcaSustain",0.8f} } },
    };
    return p;
}

inline juce::File userPresetFolder()
{
    auto f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                 .getChildFile("Ticco Ross").getChildFile("WAZP").getChildFile("Presets");
    f.createDirectory();
    return f;
}

} // namespace wazp
