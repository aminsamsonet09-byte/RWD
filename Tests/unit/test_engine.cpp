// RWD COLOR LAB - UNIT TESTS for the platform-independent CPU engine.
//
// These are *unit tests*: they exercise the shared C++ engine only. They say
// nothing about OFX registration or about DaVinci Resolve (see
// Documentation/TEST_REPORT.md for the three separate test categories).

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "mini_test.h"
#include "render/cpu/cpu_renderer.h"

using namespace rwd;

namespace {

// ---- helpers ----------------------------------------------------------------

inline PixelFormat format_of(float) { return PixelFormat::Float32; }
inline PixelFormat format_of(std::uint16_t) { return PixelFormat::UInt16; }
inline PixelFormat format_of(std::uint8_t) { return PixelFormat::UInt8; }

// Owning RGBA test image with optional row padding and optional bottom-up
// storage (negative rowBytes). Addressing in get()/set() is computed
// independently of the engine so stride bugs cannot cancel out.
template <typename T>
struct Img {
    static constexpr std::size_t kBpp = sizeof(T) * 4;
    Bounds b;
    std::ptrdiff_t stride;
    bool bottomUp;
    std::vector<unsigned char> store;

    Img(int x1, int y1, int w, int h, std::size_t padBytes = 0, bool bottomUp_ = false,
        unsigned char fill = 0xAB)
        : b{x1, y1, x1 + w, y1 + h},
          stride(static_cast<std::ptrdiff_t>(static_cast<std::size_t>(w) * kBpp + padBytes)),
          bottomUp(bottomUp_),
          store(static_cast<std::size_t>(stride) * static_cast<std::size_t>(h), fill) {}

    int w() const { return b.width(); }
    int h() const { return b.height(); }

    unsigned char* addr(int x, int y) {
        int row = y - b.y1;
        if (bottomUp) row = h() - 1 - row;
        return store.data() + static_cast<std::size_t>(row) * static_cast<std::size_t>(stride) +
               static_cast<std::size_t>(x - b.x1) * kBpp;
    }
    T get(int x, int y, int c) {
        T v;
        std::memcpy(&v, addr(x, y) + static_cast<std::size_t>(c) * sizeof(T), sizeof(T));
        return v;
    }
    void set(int x, int y, int c, T v) {
        std::memcpy(addr(x, y) + static_cast<std::size_t>(c) * sizeof(T), &v, sizeof(T));
    }
    ImageView view() {
        ImageView v;
        v.data = bottomUp ? store.data() + static_cast<std::size_t>(h() - 1) * static_cast<std::size_t>(stride)
                          : store.data();
        v.bounds = b;
        v.rowBytes = bottomUp ? -stride : stride;
        v.format = format_of(T());
        return v;
    }
    ConstImageView cview() {
        const ImageView v = view();
        ConstImageView c;
        c.data = v.data;
        c.bounds = v.bounds;
        c.rowBytes = v.rowBytes;
        c.format = v.format;
        return c;
    }
};

float pattern_rgb(int x, int y, int c) {  // spans negative, normal and >1 values
    const int v = (x * 31 + y * 17 + c * 7) % 101;
    return static_cast<float>(v) / 40.0f - 0.7f;
}
float pattern_alpha(int x, int y) { return 0.25f + static_cast<float>((x + y) % 4) * 0.25f; }

void fill_float(Img<float>& im) {
    for (int y = im.b.y1; y < im.b.y2; ++y)
        for (int x = im.b.x1; x < im.b.x2; ++x) {
            for (int c = 0; c < 3; ++c) im.set(x, y, c, pattern_rgb(x, y, c));
            im.set(x, y, 3, pattern_alpha(x, y));
        }
}

std::uint32_t bits(float f) {
    std::uint32_t u;
    std::memcpy(&u, &f, sizeof u);
    return u;
}

GradeSettings settings(double stops, double amount = 100.0, bool bypass = false) {
    GradeSettings s;
    s.camera = 1;   // Linear Rec.2020 in
    s.output = 5;   // Linear Rec.2020 out: the engine tests below check pure linear maths
    s.exposureStops = stops;
    s.masterAmountPercent = amount;
    s.bypass = bypass;
    return s;
}

RenderResult run(Img<float>& s, Img<float>& d, const Bounds& win, const GradeSettings& gs) {
    const ConstImageView sv = s.cview();
    return render_window(&sv, d.view(), win, prepare_grade(gs));
}

// Every pixel of d (inside `win`) must equal src * gain exactly; alpha bit-exact.
void check_gain_everywhere(Img<float>& s, Img<float>& d, float gain) {
    for (int y = d.b.y1; y < d.b.y2; ++y)
        for (int x = d.b.x1; x < d.b.x2; ++x) {
            for (int c = 0; c < 3; ++c) CHECK_EQ(d.get(x, y, c), s.get(x, y, c) * gain);
            CHECK_EQ(bits(d.get(x, y, 3)), bits(s.get(x, y, 3)));
        }
}

}  // namespace

// ---- exposure ---------------------------------------------------------------

TEST(exposure_zero_stops_is_exact_identity) {
    Img<float> s(0, 0, 8, 6), d(0, 0, 8, 6);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(0.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 1.0f);
}

TEST(exposure_plus_one_stop_doubles_rgb) {
    Img<float> s(0, 0, 8, 6), d(0, 0, 8, 6);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 2.0f);
}

TEST(exposure_minus_one_stop_halves_rgb) {
    Img<float> s(0, 0, 8, 6), d(0, 0, 8, 6);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(-1.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 0.5f);
}

TEST(exposure_plus_two_stops_quadruples_rgb) {
    Img<float> s(0, 0, 8, 6), d(0, 0, 8, 6);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(2.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 4.0f);
}

TEST(exposure_fractional_stop_matches_pow2) {
    Img<float> s(0, 0, 4, 4), d(0, 0, 4, 4);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(0.5)) == RenderResult::Ok);
    const float g = static_cast<float>(std::exp2(0.5));
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            for (int c = 0; c < 3; ++c) CHECK_NEAR(d.get(x, y, c), s.get(x, y, c) * g, 1e-6);
}

TEST(exposure_range_is_clamped_and_nan_is_zero_stops) {
    CHECK_EQ(prepare_grade(settings(10.0)).gain, 16.0f);
    CHECK_EQ(prepare_grade(settings(-10.0)).gain, 0.0625f);
    CHECK_EQ(prepare_grade(settings(4.0)).gain, 16.0f);
    CHECK_EQ(prepare_grade(settings(-4.0)).gain, 0.0625f);
    CHECK_EQ(prepare_grade(settings(std::numeric_limits<double>::quiet_NaN())).gain, 1.0f);
    CHECK_EQ(prepare_grade(settings(std::numeric_limits<double>::infinity())).gain, 16.0f);
}

// ---- master amount ----------------------------------------------------------

TEST(master_amount_prepare_maps_and_clamps) {
    CHECK_EQ(prepare_grade(settings(0, 100.0)).amount, 1.0f);
    CHECK_EQ(prepare_grade(settings(0, 0.0)).amount, 0.0f);
    CHECK_EQ(prepare_grade(settings(0, 50.0)).amount, 0.5f);
    CHECK_EQ(prepare_grade(settings(0, 150.0)).amount, 1.0f);
    CHECK_EQ(prepare_grade(settings(0, -20.0)).amount, 0.0f);
    CHECK_EQ(prepare_grade(settings(0, std::numeric_limits<double>::quiet_NaN())).amount, 1.0f);
}

TEST(master_amount_blends_between_input_and_graded) {
    Img<float> s(0, 0, 8, 6), d0(0, 0, 8, 6), d50(0, 0, 8, 6), d100(0, 0, 8, 6);
    fill_float(s);
    CHECK(run(s, d0, d0.b, settings(2.0, 0.0)) == RenderResult::Ok);
    CHECK(run(s, d50, d50.b, settings(2.0, 50.0)) == RenderResult::Ok);
    CHECK(run(s, d100, d100.b, settings(2.0, 100.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d0, 1.0f);    // 0%   -> ungraded
    check_gain_everywhere(s, d100, 4.0f);  // 100% -> fully graded
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 8; ++x)
            for (int c = 0; c < 3; ++c)  // 50% of a x4 gain = x2.5 (linear blend)
                CHECK_NEAR(d50.get(x, y, c), s.get(x, y, c) * 2.5f, 1e-5);
}

// ---- bypass -----------------------------------------------------------------

TEST(bypass_output_is_bit_identical_to_input) {
    Img<float> s(0, 0, 8, 6), d(0, 0, 8, 6);
    fill_float(s);
    // include awkward values: NaN, +/-Inf, -0.0 must pass through untouched
    s.set(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
    s.set(1, 0, 1, std::numeric_limits<float>::infinity());
    s.set(2, 0, 2, -std::numeric_limits<float>::infinity());
    s.set(3, 0, 0, -0.0f);
    CHECK(run(s, d, d.b, settings(4.0, 30.0, /*bypass*/ true)) == RenderResult::Ok);
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 8; ++x)
            for (int c = 0; c < 4; ++c) CHECK_EQ(bits(d.get(x, y, c)), bits(s.get(x, y, c)));
}

TEST(bypass_works_for_integer_formats) {
    Img<std::uint8_t> s(0, 0, 5, 3), d(0, 0, 5, 3);
    Img<std::uint16_t> s16(0, 0, 5, 3), d16(0, 0, 5, 3);
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 5; ++x)
            for (int c = 0; c < 4; ++c) {
                s.set(x, y, c, static_cast<std::uint8_t>(x * 40 + y * 9 + c));
                s16.set(x, y, c, static_cast<std::uint16_t>(x * 9000 + y * 300 + c));
            }
    const ConstImageView sv = s.cview(), sv16 = s16.cview();
    CHECK(render_window(&sv, d.view(), d.b, prepare_grade(settings(3.0, 50.0, true))) == RenderResult::Ok);
    CHECK(render_window(&sv16, d16.view(), d16.b, prepare_grade(settings(3.0, 50.0, true))) == RenderResult::Ok);
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 5; ++x)
            for (int c = 0; c < 4; ++c) {
                CHECK_EQ(d.get(x, y, c), s.get(x, y, c));
                CHECK_EQ(d16.get(x, y, c), s16.get(x, y, c));
            }
}

// ---- alpha ------------------------------------------------------------------

TEST(alpha_is_preserved_bit_exact_float) {
    Img<float> s(0, 0, 6, 1), d(0, 0, 6, 1);
    const float alphas[6] = {0.0f, 0.37f, 1.0f, 5.0f, -1.0f, std::numeric_limits<float>::quiet_NaN()};
    for (int x = 0; x < 6; ++x) {
        for (int c = 0; c < 3; ++c) s.set(x, 0, c, 0.5f);
        s.set(x, 0, 3, alphas[x]);
    }
    CHECK(run(s, d, d.b, settings(3.0, 50.0)) == RenderResult::Ok);
    for (int x = 0; x < 6; ++x) CHECK_EQ(bits(d.get(x, 0, 3)), bits(alphas[x]));
}

TEST(alpha_is_preserved_integer_formats) {
    Img<std::uint8_t> s8(0, 0, 4, 1), d8(0, 0, 4, 1);
    Img<std::uint16_t> s16(0, 0, 4, 1), d16(0, 0, 4, 1);
    const std::uint8_t a8[4] = {0, 1, 128, 255};
    const std::uint16_t a16[4] = {0, 1, 32768, 65535};
    for (int x = 0; x < 4; ++x) {
        for (int c = 0; c < 3; ++c) { s8.set(x, 0, c, 100); s16.set(x, 0, c, 20000); }
        s8.set(x, 0, 3, a8[x]);
        s16.set(x, 0, 3, a16[x]);
    }
    const ConstImageView v8 = s8.cview(), v16 = s16.cview();
    CHECK(render_window(&v8, d8.view(), d8.b, prepare_grade(settings(2.0))) == RenderResult::Ok);
    CHECK(render_window(&v16, d16.view(), d16.b, prepare_grade(settings(2.0))) == RenderResult::Ok);
    for (int x = 0; x < 4; ++x) {
        CHECK_EQ(d8.get(x, 0, 3), a8[x]);
        CHECK_EQ(d16.get(x, 0, 3), a16[x]);
    }
}

// ---- negative / non-finite values ------------------------------------------

TEST(negative_rgb_is_scaled_not_clipped) {
    Img<float> s(0, 0, 3, 1), d(0, 0, 3, 1);
    const float in[3] = {-0.5f, -2.0f, -0.001f};
    for (int x = 0; x < 3; ++x) {
        for (int c = 0; c < 3; ++c) s.set(x, 0, c, in[x]);
        s.set(x, 0, 3, 1.0f);
    }
    CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
    for (int x = 0; x < 3; ++x)
        for (int c = 0; c < 3; ++c) CHECK_EQ(d.get(x, 0, c), in[x] * 2.0f);
}

TEST(nan_and_infinity_never_propagate) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    const double stopsList[3] = {0.0, 4.0, -4.0};
    const double amountList[3] = {100.0, 50.0, 0.0};
    for (double stops : stopsList)
        for (double amount : amountList) {
            Img<float> s(0, 0, 4, 1), d(0, 0, 4, 1);
            const float vals[4] = {nan, inf, -inf, 1.0f};
            for (int x = 0; x < 4; ++x) {
                for (int c = 0; c < 3; ++c) s.set(x, 0, c, vals[x]);
                s.set(x, 0, 3, 1.0f);
            }
            CHECK(run(s, d, d.b, settings(stops, amount)) == RenderResult::Ok);
            for (int x = 0; x < 4; ++x)
                for (int c = 0; c < 3; ++c) CHECK(std::isfinite(d.get(x, 0, c)));
            if (amount == 100.0) {
                // Inf is first limited to +/-kPixelAbsLimit, then graded, then limited again.
                const float gain = prepare_grade(settings(stops)).gain;
                CHECK_EQ(d.get(0, 0, 0), 0.0f);                                   // NaN  -> 0
                CHECK_EQ(d.get(1, 0, 0), sanitize_pixel(kPixelAbsLimit * gain));  // +Inf -> huge, finite
                CHECK_EQ(d.get(2, 0, 0), sanitize_pixel(-kPixelAbsLimit * gain)); // -Inf -> -huge, finite
            }
        }
}

// ---- geometry: sizes, strides, windows, bounds ------------------------------

TEST(different_image_dimensions) {
    const int dims[6][2] = {{1, 1}, {1, 9}, {17, 1}, {3, 5}, {64, 48}, {257, 129}};
    for (const auto& dm : dims) {
        Img<float> s(0, 0, dm[0], dm[1]), d(0, 0, dm[0], dm[1]);
        fill_float(s);
        CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
        check_gain_everywhere(s, d, 2.0f);
    }
}

TEST(row_padding_is_respected_and_not_overwritten) {
    const std::size_t padSrc = 24, padDst = 37;  // deliberately odd (unaligned) padding
    Img<float> s(0, 0, 7, 5, padSrc), d(0, 0, 7, 5, padDst);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 2.0f);
    // padding bytes at the end of every dst row must still hold the 0xAB sentinel
    const std::size_t rowSpan = 7 * Img<float>::kBpp;
    for (int row = 0; row < 5; ++row)
        for (std::size_t i = 0; i < padDst; ++i)
            CHECK_EQ(d.store[static_cast<std::size_t>(row) * static_cast<std::size_t>(d.stride) + rowSpan + i],
                     static_cast<unsigned char>(0xAB));
}

TEST(negative_row_bytes_bottom_up_storage) {
    for (int mode = 0; mode < 3; ++mode) {
        const bool srcUp = (mode == 0 || mode == 2);
        const bool dstUp = (mode == 1 || mode == 2);
        Img<float> s(0, 0, 9, 7, 8, srcUp), d(0, 0, 9, 7, 16, dstUp);
        fill_float(s);
        CHECK(run(s, d, d.b, settings(-1.0)) == RenderResult::Ok);
        check_gain_everywhere(s, d, 0.5f);
    }
}

TEST(render_window_only_modifies_pixels_inside_window) {
    Img<float> s(0, 0, 20, 20), d(0, 0, 20, 20);
    fill_float(s);
    const std::vector<unsigned char> before = d.store;
    const Bounds win{5, 4, 14, 11};
    CHECK(run(s, d, win, settings(1.0)) == RenderResult::Ok);
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x) {
            const bool inside = x >= win.x1 && x < win.x2 && y >= win.y1 && y < win.y2;
            if (inside) {
                for (int c = 0; c < 3; ++c) CHECK_EQ(d.get(x, y, c), s.get(x, y, c) * 2.0f);
            } else {
                CHECK(std::memcmp(d.addr(x, y), before.data() + (d.addr(x, y) - d.store.data()),
                                  Img<float>::kBpp) == 0);
            }
        }
}

TEST(window_larger_than_destination_is_clipped) {
    Img<float> s(0, 0, 8, 8), d(0, 0, 8, 8);
    fill_float(s);
    CHECK(run(s, d, Bounds{-10, -10, 100, 100}, settings(1.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 2.0f);
}

TEST(non_zero_origin_bounds_are_mapped_correctly) {
    Img<float> s(-5, 3, 12, 6, 4), d(-5, 3, 12, 6);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(2.0)) == RenderResult::Ok);
    check_gain_everywhere(s, d, 4.0f);
}

TEST(source_smaller_than_destination_clears_uncovered_pixels) {
    Img<float> s(2, 3, 5, 5), d(0, 0, 10, 10);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x) {
            const bool covered = x >= 2 && x < 7 && y >= 3 && y < 8;
            for (int c = 0; c < 4; ++c) {
                if (covered) {
                    if (c < 3) CHECK_EQ(d.get(x, y, c), s.get(x, y, c) * 2.0f);
                    else CHECK_EQ(bits(d.get(x, y, 3)), bits(s.get(x, y, 3)));
                } else {
                    CHECK_EQ(bits(d.get(x, y, c)), 0u);  // black, transparent
                }
            }
        }
}

TEST(source_partially_outside_destination_on_all_sides) {
    Img<float> s(-3, -2, 9, 7), d(0, 0, 10, 10);
    fill_float(s);
    CHECK(run(s, d, d.b, settings(1.0)) == RenderResult::Ok);
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x) {
            const bool covered = x >= -3 && x < 6 && y >= -2 && y < 5;
            for (int c = 0; c < 3; ++c)
                CHECK_EQ(d.get(x, y, c), covered ? pattern_rgb(x, y, c) * 2.0f : 0.0f);
        }
}

TEST(missing_source_clears_destination) {
    Img<float> d(0, 0, 6, 4);
    CHECK(render_window(nullptr, d.view(), d.b, prepare_grade(settings(1.0))) == RenderResult::Ok);
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 6; ++x)
            for (int c = 0; c < 4; ++c) CHECK_EQ(bits(d.get(x, y, c)), 0u);
}

// ---- integer pixel formats --------------------------------------------------

TEST(uint8_exposure_rounds_and_clamps) {
    Img<std::uint8_t> s(0, 0, 4, 1), d(0, 0, 4, 1);
    const std::uint8_t in[4] = {0, 64, 128, 255};
    for (int x = 0; x < 4; ++x) {
        for (int c = 0; c < 3; ++c) s.set(x, 0, c, in[x]);
        s.set(x, 0, 3, 255);
    }
    const ConstImageView sv = s.cview();
    CHECK(render_window(&sv, d.view(), d.b, prepare_grade(settings(1.0))) == RenderResult::Ok);
    const std::uint8_t up[4] = {0, 128, 255, 255};  // 128 and 255 clamp at white
    for (int x = 0; x < 4; ++x) CHECK_EQ(d.get(x, 0, 0), up[x]);
    CHECK(render_window(&sv, d.view(), d.b, prepare_grade(settings(-1.0))) == RenderResult::Ok);
    const std::uint8_t down[4] = {0, 32, 64, 128};
    for (int x = 0; x < 4; ++x) CHECK_EQ(d.get(x, 0, 1), down[x]);
}

TEST(uint16_exposure_rounds_and_clamps) {
    Img<std::uint16_t> s(0, 0, 3, 1), d(0, 0, 3, 1);
    const std::uint16_t in[3] = {0, 16384, 40000};
    for (int x = 0; x < 3; ++x) {
        for (int c = 0; c < 3; ++c) s.set(x, 0, c, in[x]);
        s.set(x, 0, 3, 65535);
    }
    const ConstImageView sv = s.cview();
    CHECK(render_window(&sv, d.view(), d.b, prepare_grade(settings(1.0))) == RenderResult::Ok);
    CHECK_EQ(d.get(0, 0, 0), 0);
    CHECK_EQ(d.get(1, 0, 0), 32768);
    CHECK_EQ(d.get(2, 0, 0), 65535);  // 80000 clamps
}

// ---- robustness -------------------------------------------------------------

TEST(invalid_arguments_are_rejected_without_touching_memory) {
    Img<float> s(0, 0, 4, 4), d(0, 0, 4, 4);
    fill_float(s);
    const std::vector<unsigned char> before = d.store;
    const ConstImageView sv = s.cview();

    ImageView bad = d.view();
    bad.data = nullptr;
    CHECK(render_window(&sv, bad, d.b, prepare_grade(settings(1.0))) == RenderResult::InvalidArguments);

    ImageView empty = d.view();
    empty.bounds = Bounds{0, 0, 0, 0};
    CHECK(render_window(&sv, empty, d.b, prepare_grade(settings(1.0))) == RenderResult::InvalidArguments);

    ImageView overlapping = d.view();
    overlapping.rowBytes = 8;  // smaller than one row of pixels -> rows would overlap
    CHECK(render_window(&sv, overlapping, d.b, prepare_grade(settings(1.0))) == RenderResult::InvalidArguments);

    Img<std::uint8_t> d8(0, 0, 4, 4);
    CHECK(render_window(&sv, d8.view(), d8.b, prepare_grade(settings(1.0))) == RenderResult::FormatMismatch);

    CHECK(d.store == before);
}

namespace {
struct AbortAfter {
    int rowsAllowed;
    int polls = 0;
    static bool poll(void* p) {
        AbortAfter* a = static_cast<AbortAfter*>(p);
        return a->polls++ >= a->rowsAllowed;
    }
};
}  // namespace

TEST(abort_check_stops_rendering_between_rows) {
    Img<float> s(0, 0, 4, 10), d(0, 0, 4, 10);
    fill_float(s);
    AbortAfter ab{3};
    AbortCheck chk;
    chk.fn = &AbortAfter::poll;
    chk.user = &ab;
    const ConstImageView sv = s.cview();
    CHECK(render_window(&sv, d.view(), d.b, prepare_grade(settings(1.0)), chk) == RenderResult::Ok);
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 4; ++x) {
            if (y < 3) CHECK_EQ(d.get(x, y, 0), s.get(x, y, 0) * 2.0f);
            else CHECK_EQ(bits(d.get(x, y, 0)), 0xABABABABu);  // untouched sentinel
        }
}

TEST(in_place_rendering_with_aliased_buffers) {
    Img<float> im(0, 0, 8, 6), ref(0, 0, 8, 6);
    fill_float(im);
    fill_float(ref);
    const ConstImageView sv = im.cview();
    CHECK(render_window(&sv, im.view(), im.b, prepare_grade(settings(1.0))) == RenderResult::Ok);
    check_gain_everywhere(ref, im, 2.0f);
}

TEST(disjoint_windows_compose_to_the_full_render) {
    // simulates the host splitting the frame between threads
    Img<float> s(0, 0, 13, 11), whole(0, 0, 13, 11), parts(0, 0, 13, 11);
    fill_float(s);
    CHECK(run(s, whole, whole.b, settings(1.5, 70.0)) == RenderResult::Ok);
    const int cuts[4] = {0, 3, 7, 11};
    for (int i = 0; i < 3; ++i)
        CHECK(run(s, parts, Bounds{0, cuts[i], 13, cuts[i + 1]}, settings(1.5, 70.0)) == RenderResult::Ok);
    CHECK(whole.store == parts.store);
}
