// RWD COLOR LAB - camera transfer functions (encoded value 0..1 -> scene-linear, 0.18 = mid grey).
// Constants come from manufacturer documents or published reference implementations; see
// Documentation/CAMERA_PROFILES.md for the status of each curve. Decoders preserve negative values
// (linear toe segments) and never return NaN for finite input.
#pragma once

namespace rwd {

enum class Transfer {
    Gamma24,      // pure power 2.4 display decode (sign-symmetric)
    Linear,       // no transfer
    SLog2, SLog3, DLog, CLog, CLog3, VLog, FLog, FLog2, NLog, BmdFilmGen5
};

double decode_transfer(Transfer t, double encoded);

}  // namespace rwd
