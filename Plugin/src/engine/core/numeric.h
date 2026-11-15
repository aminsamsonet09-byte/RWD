// RWD COLOR LAB - shared numeric helpers.
// Header-only, no OpenFX dependency: this code is the platform-independent
// reference engine shared by the Windows, macOS (and Linux dev) builds.
//
// IMPORTANT: the NaN/Inf handling below relies on IEEE-754 semantics.
// Never compile this code with -ffast-math, -Ofast, /fp:fast or similar.
#pragma once

namespace rwd {

// Largest magnitude any RGB value may have after sanitising. Chosen so that the
// maximum exposure gain (2^4 = 16) can never overflow a 32-bit float.
inline constexpr float kPixelAbsLimit = 1.0e30f;

// NaN -> 0, +/-Inf and huge values -> +/-kPixelAbsLimit. Finite in-range values
// (including negative values) are returned unchanged.
inline float sanitize_pixel(float v) noexcept {
    if (!(v == v)) return 0.0f;                       // NaN
    if (v > kPixelAbsLimit) return kPixelAbsLimit;    // +Inf / overflow
    if (v < -kPixelAbsLimit) return -kPixelAbsLimit;  // -Inf / overflow
    return v;
}

// Clamp to [0,1]; NaN maps to 0.
inline float clamp01(float v) noexcept {
    if (!(v > 0.0f)) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

inline float mix(float a, float b, float t) noexcept { return a + (b - a) * t; }

}  // namespace rwd
