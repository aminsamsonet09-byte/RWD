// RWD COLOR LAB - exposure (working-space primary adjustment).
//
// Phase 2 definition: gain = 2^stops, applied as a plain multiplication of the
// RGB values. This is only photographically meaningful on scene-linear data.
// Until the camera-decode stage exists (Phase 3) the plug-in cannot know whether
// the host hands it linear or encoded values, so it multiplies what it receives
// (see Documentation/README.md, "Known limitations").
#pragma once

#include <cmath>

namespace rwd {

inline constexpr double kExposureMinStops = -4.0;
inline constexpr double kExposureMaxStops = 4.0;
inline constexpr double kExposureDefaultStops = 0.0;

// NaN -> default (0 stops); out-of-range values are clamped to [-4, +4].
inline double clamp_exposure_stops(double stops) noexcept {
    if (!(stops == stops)) return kExposureDefaultStops;
    if (stops < kExposureMinStops) return kExposureMinStops;
    if (stops > kExposureMaxStops) return kExposureMaxStops;
    return stops;
}

// Computed in double so that integer stop values give exactly 2^n on every
// platform/C runtime, then rounded once to float.
inline float exposure_gain(double stops) noexcept {
    return static_cast<float>(std::exp2(clamp_exposure_stops(stops)));
}

}  // namespace rwd
