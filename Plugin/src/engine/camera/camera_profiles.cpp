#include "engine/camera/camera_profiles.h"

namespace rwd {

namespace {
using S = ProfileStatus;
const CameraProfile kProfiles[kCameraProfileCount] = {
    {"Rec.709 Gamma 2.4 (no camera conversion)", Transfer::Gamma24, Gamut::Rec709, S::Verified, "Standard display-referred footage."},
    {"Linear Rec.2020 (EXR / linear input)", Transfer::Linear, Gamut::Rec2020, S::Verified, "Working space, no conversion."},
    {"Sony S-Log2 / S-Gamut", Transfer::SLog2, Gamut::SGamut, S::Reference, "Legal-range 10-bit code mapping assumed."},
    {"Sony S-Log2 / S-Gamut3.Cine", Transfer::SLog2, Gamut::SGamut3Cine, S::Reference, "Legal-range 10-bit code mapping assumed."},
    {"Sony S-Log3 / S-Gamut3", Transfer::SLog3, Gamut::SGamut, S::Verified, "Curve from Sony technical summary."},
    {"Sony S-Log3 / S-Gamut3.Cine", Transfer::SLog3, Gamut::SGamut3Cine, S::Verified, "Curve from Sony technical summary."},
    {"DJI D-Log / D-Gamut", Transfer::DLog, Gamut::DGamut, S::Verified, "DJI whitepaper."},
    {"DJI D-Log M (unavailable: no public spec)", Transfer::Linear, Gamut::Rec2020, S::Unavailable, "Not identical to D-Log; no public specification."},
    {"Canon C-Log / Cinema Gamut (experimental)", Transfer::CLog, Gamut::CinemaGamut, S::Experimental, "Gamut white point unconfirmed."},
    {"Canon C-Log3 / Cinema Gamut (experimental)", Transfer::CLog3, Gamut::CinemaGamut, S::Experimental, "Gamut white point unconfirmed."},
    {"Canon C-Log2 (unavailable: constants unconfirmed)", Transfer::Linear, Gamut::Rec2020, S::Unavailable, "Constants could not be confirmed."},
    {"Panasonic V-Log / V-Gamut", Transfer::VLog, Gamut::VGamut, S::Reference, "Matrix cross-checked with Panasonic document."},
    {"Fujifilm F-Log / F-Gamut", Transfer::FLog, Gamut::Rec2020, S::Verified, "Fujifilm F-Log data sheet; F-Gamut = BT.2020 primaries."},
    {"Fujifilm F-Log2 / F-Gamut", Transfer::FLog2, Gamut::Rec2020, S::Reference, "Matches published check values."},
    {"Nikon N-Log / BT.2020", Transfer::NLog, Gamut::Rec2020, S::Verified, "Nikon N-Log specification."},
    {"Blackmagic Film Gen 5 / Wide Gamut (experimental)", Transfer::BmdFilmGen5, Gamut::BmdWideGamut5, S::Experimental, "Curve checked; gamut primaries not cross-checked."},
};
}  // namespace

int clamp_camera_index(int i) { return (i < 0 || i >= kCameraProfileCount) ? 0 : i; }
const CameraProfile& camera_profile(int i) { return kProfiles[clamp_camera_index(i)]; }
bool camera_is_display_default(int i) { return clamp_camera_index(i) == 0; }

}  // namespace rwd
