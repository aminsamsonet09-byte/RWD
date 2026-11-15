#include "engine/output/output_profiles.h"

#include <cmath>

namespace rwd {

namespace {
using S = ProfileStatus;
const OutputProfile kOut[kOutputProfileCount] = {
    {"Rec.709 Gamma 2.4", OutputTransfer::Gamma24, Gamut::Rec709, S::Verified},
    {"Rec.709 Gamma 2.2", OutputTransfer::Gamma22, Gamut::Rec709, S::Verified},
    {"Rec.709-A (Apple, gamma ~1.96, experimental)", OutputTransfer::Gamma1961, Gamut::Rec709, S::Experimental},
    {"sRGB", OutputTransfer::Srgb, Gamut::Rec709, S::Verified},
    {"Rec.2020 (BT.2020 OETF)", OutputTransfer::Bt2020, Gamut::Rec2020, S::Verified},
    {"Linear Rec.2020 (no encoding)", OutputTransfer::Linear, Gamut::Rec2020, S::Verified},
};
}  // namespace

int clamp_output_index(int i) { return (i < 0 || i >= kOutputProfileCount) ? 0 : i; }
const OutputProfile& output_profile(int i) { return kOut[clamp_output_index(i)]; }

double encode_output(OutputTransfer t, double x) {
    const double a = std::fabs(x);
    double y;
    switch (t) {
        case OutputTransfer::Gamma24: y = std::pow(a, 1.0 / 2.4); break;
        case OutputTransfer::Gamma22: y = std::pow(a, 1.0 / 2.2); break;
        case OutputTransfer::Gamma1961: y = std::pow(a, 1.0 / 1.961); break;
        case OutputTransfer::Srgb: y = (a <= 0.0031308) ? 12.92 * a : 1.055 * std::pow(a, 1.0 / 2.4) - 0.055; break;
        case OutputTransfer::Bt2020:
            y = (a < 0.018053968510807) ? 4.5 * a : 1.09929682680944 * std::pow(a, 0.45) - 0.09929682680944;
            break;
        case OutputTransfer::Linear: return x;
        default: return x;
    }
    return std::copysign(y, x);
}

}  // namespace rwd
