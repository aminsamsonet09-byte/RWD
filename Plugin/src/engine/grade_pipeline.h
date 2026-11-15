// RWD COLOR LAB - per-pixel pipeline.
//
//   input (host RGB)
//     -> camera decode (transfer -> linear, gamut matrix -> working linear Rec.2020)   [always on]
//     -> GRADE: exposure, white balance, contrast, shadows/highlights, saturation, Creative Look x Intensity
//     -> Master Amount: blend ungraded <-> graded (working space)
//     -> output transform (matrix to output gamut + transfer encode)                    [always on]
//
// Camera and output conversion never depend on Master Amount or Intensity. Bypass is an exact copy
// (render/cpu). If the selected camera profile is Unavailable the image passes through unchanged.
#pragma once

#include <memory>

#include "engine/camera/camera_profiles.h"
#include "engine/core/numeric.h"
#include "engine/looks/look_lut.h"
#include "engine/looks/look_recipes.h"
#include "engine/output/output_profiles.h"
#include "engine/working/exposure.h"
#include "engine/working/grade_ops.h"

namespace rwd {

inline constexpr double kMasterAmountMinPercent = 0.0;
inline constexpr double kMasterAmountMaxPercent = 100.0;
inline constexpr double kMasterAmountDefaultPercent = 100.0;
inline constexpr int kLookChoiceCount = kLookCount + 1;   // 0 = None

struct GradeSettings {
    int camera = 0;                  // index into camera profiles
    int output = 0;                  // index into output profiles
    int look = 0;                    // 0 = none, 1..47
    double intensityPercent = 100.0; // look intensity 0..100
    double exposureStops = kExposureDefaultStops;
    double contrast = 0.0;           // -100..100
    double saturationPercent = 100.0;// 0..200
    double temperature = 0.0;        // -100..100 (creative warm/cool, not Kelvin)
    double tint = 0.0;               // -100..100 (green/magenta)
    double highlights = 0.0;         // -100..100
    double shadows = 0.0;            // -100..100
    double masterAmountPercent = kMasterAmountDefaultPercent;
    bool bypass = false;
};

struct PreparedGrade {
    // host-visible semantics
    bool bypass = false;      // exact copy (also set for Unavailable camera profiles)
    bool unavailable = false;
    float gain = 1.0f;        // exposure gain 2^EV
    float amount = 1.0f;      // master amount 0..1
    // conversion
    bool convIdentity = true; // default Rec.709 g2.4 in/out pair: conversion is skipped when no grade is active
    Transfer inTransfer = Transfer::Gamma24;
    OutputTransfer outTransfer = OutputTransfer::Gamma24;
    float toWorking[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    float fromWorking[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    bool inMatrixIdentity = false, outMatrixIdentity = false;
    // grade
    bool gradeActive = false;
    bool hasWb = false, hasContrast = false, hasSatOp = false, hasShHi = false;
    float wb[3] = {1, 1, 1};
    float contrastPow = 1.0f, sat = 1.0f, shStops = 0.0f, hiStops = 0.0f;
    float lookIntensity = 0.0f;
    std::shared_ptr<const Lut3D> lut;
};

PreparedGrade prepare_grade(const GradeSettings& s);

// Text for a persistent on-screen warning ("" when there is nothing to warn about).
const char* profile_warning(const GradeSettings& s) noexcept;

// True when render_window() reproduces the input exactly so the host can skip rendering.
inline bool grade_is_noop(const PreparedGrade& p) noexcept {
    return p.bypass || (p.convIdentity && !(p.gradeActive && p.amount > 0.0f));
}

namespace detail {
inline void mul3(const float m[9], float c[3]) {
    const float r = m[0] * c[0] + m[1] * c[1] + m[2] * c[2];
    const float g = m[3] * c[0] + m[4] * c[1] + m[5] * c[2];
    const float b = m[6] * c[0] + m[7] * c[1] + m[8] * c[2];
    c[0] = r; c[1] = g; c[2] = b;
}
}  // namespace detail

// Processes one RGB triple. Input sanitised (NaN -> 0, Inf -> +/-1e30); output always finite.
inline void grade_rgb(const PreparedGrade& p, float& r, float& g, float& b) noexcept {
    float c[3] = {sanitize_pixel(r), sanitize_pixel(g), sanitize_pixel(b)};
    const bool grading = p.gradeActive && p.amount > 0.0f;
    if (p.convIdentity && !grading) { r = c[0]; g = c[1]; b = c[2]; return; }

    // camera decode
    for (int i = 0; i < 3; ++i) c[i] = sanitize_pixel(static_cast<float>(decode_transfer(p.inTransfer, c[i])));
    if (!p.inMatrixIdentity) detail::mul3(p.toWorking, c);

    if (grading) {
        const float u[3] = {c[0], c[1], c[2]};
        float v[3] = {c[0] * p.gain, c[1] * p.gain, c[2] * p.gain};
        if (p.hasWb) { v[0] *= p.wb[0]; v[1] *= p.wb[1]; v[2] *= p.wb[2]; }
        if (p.hasContrast) op_contrast(v, p.contrastPow);
        if (p.hasShHi) op_shadows_highlights(v, p.shStops, p.hiStops);
        if (p.hasSatOp) op_saturation(v, p.sat);
        if (p.lookIntensity > 0.0f && p.lut) {
            float l[3] = {v[0], v[1], v[2]};
            apply_lut(*p.lut, l);
            for (int i = 0; i < 3; ++i) v[i] = mix(v[i], l[i], p.lookIntensity);
        }
        for (int i = 0; i < 3; ++i) c[i] = sanitize_pixel(p.amount >= 1.0f ? v[i] : mix(u[i], v[i], p.amount));
    }

    // output transform
    if (!p.outMatrixIdentity) detail::mul3(p.fromWorking, c);
    for (int i = 0; i < 3; ++i) c[i] = static_cast<float>(encode_output(p.outTransfer, c[i]));
    r = sanitize_pixel(c[0]); g = sanitize_pixel(c[1]); b = sanitize_pixel(c[2]);
}

}  // namespace rwd
