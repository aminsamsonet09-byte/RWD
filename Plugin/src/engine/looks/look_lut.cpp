#include "engine/looks/look_lut.h"

#include <array>
#include <cmath>
#include <mutex>

#include "engine/looks/look_recipes.h"

namespace rwd {

namespace {
constexpr float kShaperFloor = 1.0f / 1024.0f;
constexpr float kShaperMax = 64.0f;
}

float shaper_forward(float v) {
    if (!(v > 0)) return 0.0f;
    const float u = (std::log2(v + kShaperFloor) + 10.0f) / 16.0f;
    return u < 0 ? 0.0f : (u > 1 ? 1.0f : u);
}
float shaper_inverse(float u) { return std::exp2(u * 16.0f - 10.0f) - kShaperFloor; }

std::shared_ptr<const Lut3D> bake_look_lut(int look) {
    auto lut = std::make_shared<Lut3D>();
    const int N = kLutSize;
    lut->data.resize(static_cast<size_t>(N) * N * N * 3);
    const LookRecipe& rec = look_recipe(look);
    float lin[kLutSize];
    for (int i = 0; i < N; ++i) {
        const float v = shaper_inverse(static_cast<float>(i) / (N - 1));
        lin[i] = i == 0 ? 0.0f : v;
    }
    size_t o = 0;
    for (int b = 0; b < N; ++b)
        for (int g = 0; g < N; ++g)
            for (int r = 0; r < N; ++r) {
                float c[3] = {lin[r], lin[g], lin[b]};
                apply_recipe(rec, c);
                for (int k = 0; k < 3; ++k) lut->data[o++] = std::isfinite(c[k]) ? c[k] : 0.0f;
            }
    return lut;
}

std::shared_ptr<const Lut3D> cached_look_lut(int look) {
    static std::mutex mu;
    static std::array<std::shared_ptr<const Lut3D>, kLookCount> cache;
    if (look < 0 || look >= kLookCount) return nullptr;
    {
        std::lock_guard<std::mutex> lock(mu);
        if (cache[look]) return cache[look];
    }
    auto baked = bake_look_lut(look);  // outside the lock; a rare duplicate bake is harmless
    std::lock_guard<std::mutex> lock(mu);
    if (!cache[look]) cache[look] = baked;
    return cache[look];
}

void apply_lut(const Lut3D& lut, float rgb[3]) {
    const int N = lut.size;
    float p[3], extra[3];
    for (int k = 0; k < 3; ++k) {
        const float v = rgb[k];
        const float pos = v > 0 ? v : 0.0f;
        const float cl = pos < kShaperMax ? pos : kShaperMax;
        extra[k] = (v - cl);  // negative part (v<0) or overflow (v>64), carried additively
        p[k] = shaper_forward(cl) * (N - 1);
    }
    int i0[3];
    float f[3];
    for (int k = 0; k < 3; ++k) {
        int i = static_cast<int>(p[k]);
        if (i > N - 2) i = N - 2;
        i0[k] = i;
        f[k] = p[k] - static_cast<float>(i);
    }
    auto at = [&](int r, int g, int b) { return &lut.data[(static_cast<size_t>(b) * N + g) * N * 3 + static_cast<size_t>(r) * 3]; };
    const float *c000 = at(i0[0], i0[1], i0[2]), *c111 = at(i0[0] + 1, i0[1] + 1, i0[2] + 1);
    const float *ca, *cb;  // the two intermediate tetrahedron vertices
    float w1, w2, w3;      // weights of c000 handled as 1-w1, then ca, cb, c111
    const float fr = f[0], fg = f[1], fb = f[2];
    if (fr >= fg) {
        if (fg >= fb) { ca = at(i0[0] + 1, i0[1], i0[2]); cb = at(i0[0] + 1, i0[1] + 1, i0[2]); w1 = fr; w2 = fg; w3 = fb; }
        else if (fr >= fb) { ca = at(i0[0] + 1, i0[1], i0[2]); cb = at(i0[0] + 1, i0[1], i0[2] + 1); w1 = fr; w2 = fb; w3 = fg; }
        else { ca = at(i0[0], i0[1], i0[2] + 1); cb = at(i0[0] + 1, i0[1], i0[2] + 1); w1 = fb; w2 = fr; w3 = fg; }
    } else {
        if (fr >= fb) { ca = at(i0[0], i0[1] + 1, i0[2]); cb = at(i0[0] + 1, i0[1] + 1, i0[2]); w1 = fg; w2 = fr; w3 = fb; }
        else if (fg >= fb) { ca = at(i0[0], i0[1] + 1, i0[2]); cb = at(i0[0], i0[1] + 1, i0[2] + 1); w1 = fg; w2 = fb; w3 = fr; }
        else { ca = at(i0[0], i0[1], i0[2] + 1); cb = at(i0[0], i0[1] + 1, i0[2] + 1); w1 = fb; w2 = fg; w3 = fr; }
    }
    // order: c000 -> ca -> cb -> c111 with decreasing weights w1 >= w2 >= w3
    for (int k = 0; k < 3; ++k)
        rgb[k] = (1.0f - w1) * c000[k] + (w1 - w2) * ca[k] + (w2 - w3) * cb[k] + w3 * c111[k] + extra[k];
}

}  // namespace rwd
