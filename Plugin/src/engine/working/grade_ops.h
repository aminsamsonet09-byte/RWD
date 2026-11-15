// RWD COLOR LAB - basic grading operations in scene-linear Rec.2020 working space.
#pragma once
#include <cmath>

namespace rwd {

inline constexpr float kLumaR = 0.2627f, kLumaG = 0.6780f, kLumaB = 0.0593f;  // Rec.2020 luma
inline constexpr float kMidGrey = 0.18f;

inline float luma2020(const float c[3]) { return kLumaR * c[0] + kLumaG * c[1] + kLumaB * c[2]; }

// Contrast: power about the mid-grey pivot on luma ratio (hue preserving). pow = 1 -> identity.
inline void op_contrast(float c[3], float power) {
    const float Y = luma2020(c);
    if (Y > 1e-6f) {
        const float k = kMidGrey * std::pow(Y / kMidGrey, power) / Y;
        c[0] *= k; c[1] *= k; c[2] *= k;
    }
}
// Shadows / highlights: smooth logistic masks over stops about mid grey; strictly monotonic for |amt|<=1.
inline void op_shadows_highlights(float c[3], float shadowsStops, float highlightsStops) {
    const float Y = luma2020(c);
    if (!(Y > 1e-6f)) return;
    const float s = std::log2(Y / kMidGrey);
    const float ms = 1.0f / (1.0f + std::exp(1.2f * (s + 1.0f)));
    const float mh = 1.0f / (1.0f + std::exp(-1.2f * (s - 1.0f)));
    const float k = std::exp2(shadowsStops * ms + highlightsStops * mh);
    c[0] *= k; c[1] *= k; c[2] *= k;
}
inline void op_saturation(float c[3], float sat) {
    const float Y = luma2020(c);
    c[0] = Y + (c[0] - Y) * sat; c[1] = Y + (c[1] - Y) * sat; c[2] = Y + (c[2] - Y) * sat;
}

}  // namespace rwd
