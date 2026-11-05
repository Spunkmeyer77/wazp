#pragma once
// Note divisions for the control oscillator when it is synced to the host tempo.
#include <cmath>
#include <algorithm>

namespace wazp::sync
{
inline constexpr int numDivisions = 15;
inline const char* const names[numDivisions] = {
    "4 BARS", "2 BARS", "1 BAR", "1/2", "1/2T", "1/4.", "1/4", "1/4T", "1/8.", "1/8", "1/8T", "1/16.", "1/16", "1/16T", "1/32" };
// length of one control-osc cycle in quarter notes
inline const double beats[numDivisions] = {
    16.0, 8.0, 4.0, 2.0, 4.0 / 3.0, 1.5, 1.0, 2.0 / 3.0, 0.75, 0.5, 1.0 / 3.0, 0.375, 0.25, 1.0 / 6.0, 0.125 };

// the FREQ. knob position (0..1) picks the division: slow on the left, fast on the right
inline int divisionFromKnob(float norm)
{
    return std::clamp((int) std::floor(norm * (float) numDivisions), 0, numDivisions - 1);
}
}
