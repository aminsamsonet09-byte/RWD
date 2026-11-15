// RWD COLOR LAB - output transforms (working linear Rec.2020 -> display/encoded). Append-only index.
#pragma once
#include "engine/camera/camera_profiles.h"

namespace rwd {

enum class OutputTransfer { Gamma24, Gamma22, Gamma1961, Srgb, Bt2020, Linear };

struct OutputProfile {
    const char* label;
    OutputTransfer transfer;
    Gamut gamut;
    ProfileStatus status;
};

constexpr int kOutputProfileCount = 6;
const OutputProfile& output_profile(int index);   // out-of-range -> entry 0
int clamp_output_index(int index);
double encode_output(OutputTransfer t, double linear);  // sign-symmetric, finite for finite input

}  // namespace rwd
