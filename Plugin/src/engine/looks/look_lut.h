// RWD COLOR LAB - baked 65^3 look LUTs with a log shaper and tetrahedral interpolation.
#pragma once
#include <memory>
#include <vector>

namespace rwd {

constexpr int kLutSize = 65;

struct Lut3D {
    std::vector<float> data;  // size^3 * 3, index ((b*N + g)*N + r)*3, red fastest
    int size = kLutSize;
};

// Shaper: u = (log2(v + 2^-10) + 10) / 16, v in [0, 64). u -> v inverse provided for baking.
float shaper_forward(float v);
float shaper_inverse(float u);

std::shared_ptr<const Lut3D> bake_look_lut(int zeroBasedLook);
std::shared_ptr<const Lut3D> cached_look_lut(int zeroBasedLook);  // thread-safe, baked once per process

// Applies the LUT. Negative input is carried through additively, values > 64 extended linearly.
void apply_lut(const Lut3D& lut, float rgb[3]);

}  // namespace rwd
