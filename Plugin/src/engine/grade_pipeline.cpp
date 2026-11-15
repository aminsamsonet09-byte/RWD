#include "engine/grade_pipeline.h"

#include <cmath>

namespace rwd {

namespace {
double clampd(double v, double lo, double hi, double dflt) {
    if (!(v == v)) return dflt;
    return v < lo ? lo : (v > hi ? hi : v);
}
void to_float(const Mat3& m, float out[9]) { for (int i = 0; i < 9; ++i) out[i] = static_cast<float>(m.m[i]); }
bool is_identity(const Mat3& m) {
    const Mat3 id = mat_identity();
    for (int i = 0; i < 9; ++i) if (std::fabs(m.m[i] - id.m[i]) > 1e-7) return false;
    return true;
}
}  // namespace

PreparedGrade prepare_grade(const GradeSettings& s) {
    PreparedGrade p;
    const int ci = clamp_camera_index(s.camera);
    const int oi = clamp_output_index(s.output);
    const CameraProfile& cam = camera_profile(ci);
    const OutputProfile& out = output_profile(oi);

    p.bypass = s.bypass;
    if (cam.status == ProfileStatus::Unavailable) { p.bypass = true; p.unavailable = true; }

    p.gain = exposure_gain(s.exposureStops);
    p.amount = static_cast<float>(clampd(s.masterAmountPercent, kMasterAmountMinPercent, kMasterAmountMaxPercent,
                                         kMasterAmountDefaultPercent) / 100.0);

    p.inTransfer = cam.transfer;
    p.outTransfer = out.transfer;
    const Mat3 tw = gamut_to_working(cam.gamut), fw = working_to_gamut(out.gamut);
    to_float(tw, p.toWorking);
    to_float(fw, p.fromWorking);
    p.inMatrixIdentity = is_identity(tw);
    p.outMatrixIdentity = is_identity(fw);
    p.convIdentity = (ci == 0 && oi == 0);

    // grade operations
    const double con = clampd(s.contrast, -100, 100, 0), sat = clampd(s.saturationPercent, 0, 200, 100);
    const double temp = clampd(s.temperature, -100, 100, 0), tint = clampd(s.tint, -100, 100, 0);
    const double hi = clampd(s.highlights, -100, 100, 0), sh = clampd(s.shadows, -100, 100, 0);

    p.hasContrast = con != 0;
    p.contrastPow = static_cast<float>(std::exp2(con / 100.0));
    p.hasSatOp = sat != 100;
    p.sat = static_cast<float>(sat / 100.0);
    p.hasShHi = sh != 0 || hi != 0;
    p.shStops = static_cast<float>(sh / 100.0);    // + lifts shadows
    p.hiStops = static_cast<float>(hi / 100.0);    // + brightens highlights
    p.hasWb = temp != 0 || tint != 0;
    if (p.hasWb) {
        double g[3] = {std::exp2(temp / 100.0 * 0.35 + tint / 200.0 * 0.25), std::exp2(-tint / 100.0 * 0.25),
                       std::exp2(-temp / 100.0 * 0.35 + tint / 200.0 * 0.25)};
        const double n = kLumaR * g[0] + kLumaG * g[1] + kLumaB * g[2];   // keep luma neutral
        for (int i = 0; i < 3; ++i) p.wb[i] = static_cast<float>(g[i] / n);
    }

    const int look = (s.look < 0 || s.look > kLookCount) ? 0 : s.look;
    p.lookIntensity = static_cast<float>(clampd(s.intensityPercent, 0, 100, 100) / 100.0);
    if (look > 0 && p.lookIntensity > 0.0f) p.lut = cached_look_lut(look - 1);
    if (!p.lut) p.lookIntensity = 0.0f;

    p.gradeActive = (p.gain != 1.0f) || p.hasWb || p.hasContrast || p.hasShHi || p.hasSatOp || p.lookIntensity > 0.0f;
    return p;
}

const char* profile_warning(const GradeSettings& s) noexcept {
    const CameraProfile& cam = camera_profile(s.camera);
    const OutputProfile& out = output_profile(s.output);
    if (cam.status == ProfileStatus::Unavailable)
        return "UNAVAILABLE: no public spec for this camera profile - the image passes through unchanged.";
    if (cam.status == ProfileStatus::Experimental)
        return "Experimental camera profile: part of its definition is unconfirmed, colours may be inexact.";
    if (out.status == ProfileStatus::Experimental)
        return "Experimental output profile: its definition is unconfirmed.";
    return "";
}

}  // namespace rwd
