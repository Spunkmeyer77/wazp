#pragma once
// Panel geometry in artwork coordinates (1536 x 1024) - generated from assets/ui/layout.json
#include <juce_graphics/juce_graphics.h>

namespace wazp::layout
{
constexpr float panelW = 1536.0f, panelH = 1024.0f;

struct KnobPos { const char* artId; float x, y, s; };
inline const KnobPos knobs[] = {
    { "glide", 153.0f, 519.0f, 1.0f },
    { "osc1-range", 261.0f, 326.0f, 1.0f },
    { "osc1-width", 365.0f, 326.0f, 1.0f },
    { "osc1-wave", 477.0f, 326.0f, 1.0f },
    { "osc2-range", 261.0f, 519.0f, 1.0f },
    { "osc2-pitch", 365.0f, 519.0f, 1.0f },
    { "osc2-wave", 477.0f, 519.0f, 1.0f },
    { "lfo-frequency", 591.0f, 318.0f, 1.0f },
    { "pitch-mod", 698.0f, 318.0f, 1.0f },
    { "noise", 591.0f, 519.0f, 1.0f },
    { "lfo-wave", 698.0f, 519.0f, 1.0f },
    { "filter-frequency", 819.0f, 318.0f, 1.0f },
    { "resonance", 925.0f, 318.0f, 1.0f },
    { "filter-mode", 870.0f, 415.0f, 1.0f },
    { "filter-mod1", 819.0f, 519.0f, 1.0f },
    { "filter-mod2", 925.0f, 519.0f, 1.0f },
    { "vca-attack", 1045.0f, 318.0f, 1.0f },
    { "vca-decay", 1157.0f, 318.0f, 1.0f },
    { "vca-sustain", 1268.0f, 318.0f, 1.0f },
    { "env-attack", 1045.0f, 519.0f, 1.0f },
    { "env-decay", 1157.0f, 519.0f, 1.0f },
    { "env-delay", 1268.0f, 519.0f, 1.0f },
    { "volume", 1382.0f, 318.0f, 1.0f },
    { "knob_osc1_level", 1382.0f, 453.0f, 0.77f },
    { "knob_osc2_level", 1382.0f, 544.0f, 0.77f },
};

inline const juce::Rectangle<float> whiteKeys[] = {
    { 135.0f, 700.0f, 82.8f, 243.0f },
    { 219.3f, 700.0f, 82.8f, 243.0f },
    { 303.6f, 700.0f, 82.8f, 243.0f },
    { 387.9f, 700.0f, 82.8f, 243.0f },
    { 472.2f, 700.0f, 82.8f, 243.0f },
    { 556.5f, 700.0f, 82.8f, 243.0f },
    { 640.8f, 700.0f, 82.8f, 243.0f },
    { 725.1f, 700.0f, 82.8f, 243.0f },
    { 809.4f, 700.0f, 82.8f, 243.0f },
    { 893.6999999999999f, 700.0f, 82.8f, 243.0f },
    { 978.0f, 700.0f, 82.8f, 243.0f },
    { 1062.3f, 700.0f, 82.8f, 243.0f },
    { 1146.6f, 700.0f, 82.8f, 243.0f },
    { 1230.8999999999999f, 700.0f, 82.8f, 243.0f },
    { 1315.2f, 700.0f, 82.8f, 243.0f },
};
// black keys: index of the white key they follow, then the rectangle
struct BlackKey { int after; juce::Rectangle<float> r; };
inline const BlackKey blackKeys[] = {
    { 0, { 192.3f, 700.0f, 54.0f, 132.0f } },
    { 1, { 276.6f, 700.0f, 54.0f, 132.0f } },
    { 3, { 445.2f, 700.0f, 54.0f, 132.0f } },
    { 4, { 529.5f, 700.0f, 54.0f, 132.0f } },
    { 5, { 613.8f, 700.0f, 54.0f, 132.0f } },
    { 7, { 782.4f, 700.0f, 54.0f, 132.0f } },
    { 8, { 866.6999999999999f, 700.0f, 54.0f, 132.0f } },
    { 10, { 1035.3f, 700.0f, 54.0f, 132.0f } },
    { 11, { 1119.6f, 700.0f, 54.0f, 132.0f } },
    { 12, { 1203.8999999999999f, 700.0f, 54.0f, 132.0f } },
};

inline const juce::Rectangle<float> presetdisplay { 1125.0f, 84.0f, 234.0f, 51.0f };
inline const juce::Rectangle<float> presetprev { 1081.0f, 84.0f, 36.0f, 51.0f };
inline const juce::Rectangle<float> presetnext { 1367.0f, 84.0f, 36.0f, 51.0f };
inline const juce::Rectangle<float> presetsave { 1144.0f, 150.0f, 92.0f, 34.0f };
inline const juce::Rectangle<float> presetload { 1248.0f, 150.0f, 92.0f, 34.0f };
inline const juce::Point<float> syncSwitch { 153.0f, 318.0f };   // centre of the 68 x 46 switch cell
inline const juce::Point<float> holdSwitch { 153.0f, 410.0f };
inline const juce::Rectangle<float> pitchDestSwitch { 651.0f, 422.0f, 94.0f, 18.0f };   // OSC 1 / BOTH / OSC 2 slide
inline const juce::Rectangle<float> lfoSyncSwitch { 557.0f, 422.0f, 68.0f, 18.0f };     // FREE / SYNC slide
inline const juce::Rectangle<float> badge { 770.0f, 62.0f, 290.0f, 140.0f };   // WAZP logo -> About
}
