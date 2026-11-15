// RWD COLOR LAB - colour-science unit tests. Reference encoders here are written independently of
// the engine's decoders (straight from the documents' forward formulas / check values).
#include <cmath>
#include <string>
#include <vector>

#include "engine/grade_pipeline.h"
#include "mini_test.h"

using namespace rwd;

namespace {
double enc_slog3(double x) { return x >= 0.01125 ? (420 + std::log10((x + 0.01) / 0.19) * 261.5) / 1023 : (x * (171.2102946929 - 95) / 0.01125 + 95) / 1023; }
double enc_slog2(double r) { double x = r / 0.9 * 155 / 219; double y = x >= 0 ? 0.432699 * std::log10(x + 0.037584) + 0.616596 + 0.03 : x * 5 + 0.030001222851889303; return (y * 876 + 64) / 1023; }
double enc_flog(double x, double a, double b, double c, double d, double e, double f, double cut) { return x >= cut ? c * std::log10(a * x + b) + d : e * x + f; }
double enc_nlog(double y) { return (y < 0.328 ? 650 * std::pow(y + 0.0075, 1.0 / 3) : 150 * std::log(y) + 619) / 1023; }
double enc_dlog(double x) { return x <= 0.0078 ? 6.025 * x + 0.0929 : std::log10(x * 0.9892 + 0.0108) * 0.256663 + 0.584555; }
double enc_vlog(double x) { return x < 0.01 ? 5.6 * x + 0.125 : 0.241514 * std::log10(x + 0.00873) + 0.598206; }
double enc_clog(double r) { double x = r / 0.9; return 0.529136 * std::log10(10.1596 * x + 1) + 0.0730597; }
double enc_clog3(double r) { double x = r / 0.9; return x < -0.014 ? -0.36726845 * std::log10(-x * 14.98325 + 1) + 0.12783901 : (x <= 0.014 ? 1.9754798 * x + 0.12512219 : 0.36726845 * std::log10(x * 14.98325 + 1) + 0.12240537); }
double enc_bmd5(double x) { return x < 0.005 ? 8.283605932402494 * x + 0.09246575342465753 : 0.08692876065491224 * std::log(x + 0.005494072432257808) + 0.5300133392291939; }

template <typename F>
void roundtrip(Transfer t, F enc, double tol) {
    for (double x : {0.0, 0.002, 0.01, 0.05, 0.18, 0.5, 0.9, 2.0, 6.0})
        CHECK_NEAR(decode_transfer(t, enc(x)), x, tol * (1.0 + x));
}
}  // namespace

TEST(transfer_roundtrip_against_independent_encoders) {
    roundtrip(Transfer::SLog3, enc_slog3, 1e-9);
    roundtrip(Transfer::SLog2, enc_slog2, 1e-9);
    roundtrip(Transfer::FLog, [](double x) { return enc_flog(x, 0.555556, 0.009468, 0.344676, 0.790453, 8.735631, 0.092864, 0.00089); }, 1e-8);
    roundtrip(Transfer::FLog2, [](double x) { return enc_flog(x, 5.555556, 0.064829, 0.245281, 0.384316, 8.799461, 0.092864, 0.000889); }, 1e-8);
    roundtrip(Transfer::DLog, enc_dlog, 1e-8);
    roundtrip(Transfer::VLog, enc_vlog, 2e-6);  // Panasonic's own two segments differ by ~6e-6 at the break
    roundtrip(Transfer::CLog, enc_clog, 1e-8);
    roundtrip(Transfer::CLog3, enc_clog3, 1e-8);
    roundtrip(Transfer::BmdFilmGen5, enc_bmd5, 1e-8);
    // N-Log is continuous only to ~5e-4 in Nikon's own formulas (452 break), so use a looser tolerance
    for (double x : {0.002, 0.05, 0.18, 0.5, 0.9, 3.0}) CHECK_NEAR(decode_transfer(Transfer::NLog, enc_nlog(x)), x, 2e-3 * (1 + x));
}

TEST(documented_check_values_decode_to_mid_grey_and_white) {
    // S-Log3 codes 95 / 420 / 598 -> 0 / 0.18 / 0.90
    CHECK_NEAR(decode_transfer(Transfer::SLog3, 95.0 / 1023), 0.0, 1e-6);
    CHECK_NEAR(decode_transfer(Transfer::SLog3, 420.0 / 1023), 0.18, 1e-6);
    CHECK_NEAR(decode_transfer(Transfer::SLog3, 598.0 / 1023), 0.9, 2e-3);
    // S-Log2 codes 90 / 347 / 582
    CHECK_NEAR(decode_transfer(Transfer::SLog2, 90.0 / 1023), 0.0, 2e-3);
    CHECK_NEAR(decode_transfer(Transfer::SLog2, 347.0 / 1023), 0.18, 2e-3);
    CHECK_NEAR(decode_transfer(Transfer::SLog2, 582.0 / 1023), 0.9, 4e-3);
    // F-Log codes 95 / 470 / 705
    CHECK_NEAR(decode_transfer(Transfer::FLog, 95.0 / 1023), 0.0, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::FLog, 470.0 / 1023), 0.18, 2e-3);
    CHECK_NEAR(decode_transfer(Transfer::FLog, 705.0 / 1023), 0.9, 4e-3);
    // D-Log 95 / 408 / 586
    CHECK_NEAR(decode_transfer(Transfer::DLog, 408.0 / 1023), 0.18, 2e-3);
    CHECK_NEAR(decode_transfer(Transfer::DLog, 586.0 / 1023), 0.9, 4e-3);
    // single-value checks from the specs
    CHECK_NEAR(decode_transfer(Transfer::VLog, 0.42331), 0.18, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::CLog3, 0.34339), 0.18, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::FLog, 0.45932), 0.18, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::NLog, 0.36367), 0.18, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::BmdFilmGen5, 0.38356), 0.18, 1e-3);
    CHECK_NEAR(decode_transfer(Transfer::CLog, 0.328), 0.18, 2e-3);
}

TEST(decoders_are_monotonic_and_finite) {
    for (int t = 0; t <= static_cast<int>(Transfer::BmdFilmGen5); ++t) {
        double prev = -1e300;
        for (int i = -20; i <= 1100; ++i) {
            const double v = decode_transfer(static_cast<Transfer>(t), i / 1023.0);
            CHECK(std::isfinite(v));
            CHECK(v >= prev - 1e-3);  // N-Log has one documented 5e-4 step
            prev = v;
        }
    }
}

TEST(gamut_matrices_match_published_matrices_and_preserve_white) {
    // DJI D-Gamut -> XYZ (DJI whitepaper) via working space: compare derived XYZ matrix
    const GamutDef& d = gamut_def(Gamut::DGamut);
    const Mat3 m = rgb_to_xyz(d.prim, d.white);
    const double pub[9] = {0.6482, 0.1940, 0.1082, 0.2830, 0.8132, -0.0962, -0.0183, -0.0832, 1.1903};
    for (int i = 0; i < 9; ++i) CHECK_NEAR(m.m[i], pub[i], 2e-3);
    const GamutDef& v = gamut_def(Gamut::VGamut);
    const Mat3 mv = rgb_to_xyz(v.prim, {0.31271, 0.32902});
    const double pv[9] = {0.679644, 0.152211, 0.1186, 0.260686, 0.774894, -0.03558, -0.00931, -0.004612, 1.10298};
    for (int i = 0; i < 9; ++i) CHECK_NEAR(mv.m[i], pv[i], 2e-3);
    // white (1,1,1) stays (1,1,1) in the working space for every gamut; round trip is identity
    for (int g = 0; g <= static_cast<int>(Gamut::BmdWideGamut5); ++g) {
        const Mat3 a = gamut_to_working(static_cast<Gamut>(g)), b = working_to_gamut(static_cast<Gamut>(g));
        const double w[3] = {1, 1, 1};
        double o[3];
        mat_vec(a, w, o);
        for (int k = 0; k < 3; ++k) CHECK_NEAR(o[k], 1.0, 1e-6);
        const Mat3 id = mat_mul(b, a);
        for (int i = 0; i < 9; ++i) CHECK_NEAR(id.m[i], i % 4 == 0 ? 1.0 : 0.0, 1e-9);
    }
    const Mat3 r2020 = gamut_to_working(Gamut::Rec2020);
    for (int i = 0; i < 9; ++i) CHECK_NEAR(r2020.m[i], i % 4 == 0 ? 1.0 : 0.0, 1e-9);
}

TEST(output_transfers_known_points_and_sign_symmetry) {
    CHECK_NEAR(encode_output(OutputTransfer::Srgb, 0.18), 0.4613561, 1e-6);
    CHECK_NEAR(encode_output(OutputTransfer::Gamma24, 0.18), std::pow(0.18, 1 / 2.4), 1e-12);
    CHECK_NEAR(encode_output(OutputTransfer::Gamma22, 1.0), 1.0, 1e-12);
    CHECK_NEAR(encode_output(OutputTransfer::Bt2020, 1.0), 1.0, 1e-9);
    CHECK_NEAR(encode_output(OutputTransfer::Bt2020, 0.018), 0.081, 1e-12);
    for (int t = 0; t <= static_cast<int>(OutputTransfer::Linear); ++t)
        for (double x : {0.001, 0.2, 1.5})
            CHECK_NEAR(encode_output(static_cast<OutputTransfer>(t), -x), -encode_output(static_cast<OutputTransfer>(t), x), 1e-12);
}

TEST(profile_tables_are_consistent) {
    CHECK_EQ(kCameraProfileCount, 16);
    CHECK_EQ(kOutputProfileCount, 6);
    CHECK_EQ(kLookCount, 47);
    int counts[kCollectionCount] = {0};
    const char* names[kCollectionCount] = {"Cinematic", "Kodak-inspired", "Fujifilm-inspired", "Vintage", "Commercial / Fashion", "Mood"};
    for (int i = 0; i < kLookCount; ++i)
        for (int c = 0; c < kCollectionCount; ++c)
            if (std::string(look_recipe(i).collection) == names[c]) ++counts[c];
    const int want[kCollectionCount] = {10, 7, 6, 6, 8, 10};
    for (int c = 0; c < kCollectionCount; ++c) CHECK_EQ(counts[c], want[c]);
    for (int i = 0; i < kLookCount; ++i)
        for (int j = i + 1; j < kLookCount; ++j) CHECK(std::string(look_recipe(i).name) != look_recipe(j).name);
}

TEST(looks_are_finite_distinct_and_have_effect) {
    // 18 patches: greys and saturated colours
    std::vector<std::vector<float>> out(kLookCount);
    const float cols[6][3] = {{1, 0.1f, 0.1f}, {0.1f, 1, 0.1f}, {0.1f, 0.1f, 1}, {1, 1, 0.1f}, {0.8f, 0.5f, 0.4f}, {0.3f, 0.5f, 0.8f}};
    for (int i = 0; i < kLookCount; ++i) {
        for (float gv : {0.01f, 0.05f, 0.18f, 0.5f, 0.9f, 2.0f}) {
            float c[3] = {gv, gv, gv};
            apply_recipe(look_recipe(i), c);
            for (float v : c) { CHECK(std::isfinite(v)); out[i].push_back(std::pow(std::fabs(v) * 0.5f, 1 / 2.4f)); }
        }
        for (auto& col : cols) {
            float c[3] = {col[0] * 0.5f, col[1] * 0.5f, col[2] * 0.5f};
            apply_recipe(look_recipe(i), c);
            for (float v : c) { CHECK(std::isfinite(v)); out[i].push_back(std::pow(std::fabs(v), 1 / 2.4f)); }
        }
    }
    double minD = 1e9;
    int mi = 0, mj = 0;
    for (int i = 0; i < kLookCount; ++i)
        for (int j = i + 1; j < kLookCount; ++j) {
            double s = 0;
            for (size_t k = 0; k < out[i].size(); ++k) s += (out[i][k] - out[j][k]) * (out[i][k] - out[j][k]);
            const double rms = std::sqrt(s / out[i].size());
            if (rms < minD) { minD = rms; mi = i; mj = j; }
        }
    std::printf("    min pairwise look RMS distance = %.4f (%s vs %s)\n", minD, look_recipe(mi).name, look_recipe(mj).name);
    CHECK(minD > 0.015);
}

TEST(lut_matches_direct_evaluation_and_extends_out_of_range) {
    for (int look : {0, 12, 27, 46}) {
        auto lut = bake_look_lut(look);
        CHECK_EQ(lut->data.size(), static_cast<size_t>(65 * 65 * 65 * 3));
        for (float r : {0.0f, 0.02f, 0.18f, 0.6f, 3.0f})
            for (float g : {0.01f, 0.18f, 0.9f})
                for (float b : {0.0f, 0.1f, 1.0f, 8.0f}) {
                    float a[3] = {r, g, b}, d[3] = {r, g, b};
                    apply_lut(*lut, a);
                    apply_recipe(look_recipe(look), d);
                    for (int k = 0; k < 3; ++k) CHECK_NEAR(a[k], d[k], 0.012 * (1 + std::fabs(d[k])));
                }
        float n[3] = {-0.05f, 0.18f, 100.0f}, z[3] = {0.0f, 0.18f, 64.0f};
        apply_lut(*lut, n);
        apply_lut(*lut, z);
        for (float v : n) CHECK(std::isfinite(v));
        CHECK_NEAR(n[0], z[0] - 0.05f, 1e-6);    // negative part carried through additively
        CHECK_NEAR(n[2], z[2] + 36.0f, 1e-4);    // overflow above 64 extended, not clipped
    }
    CHECK(cached_look_lut(5).get() == cached_look_lut(5).get());   // cache returns the same bake
    CHECK(cached_look_lut(-1) == nullptr);
}

namespace {
void run3(const GradeSettings& s, float& r, float& g, float& b) { grade_rgb(prepare_grade(s), r, g, b); }
}

TEST(default_settings_are_exact_identity_and_noop) {
    GradeSettings s;
    const PreparedGrade p = prepare_grade(s);
    CHECK(grade_is_noop(p));
    float r = 0.3f, g = 0.6f, b = 0.9f;
    grade_rgb(p, r, g, b);
    CHECK_EQ(r, 0.3f); CHECK_EQ(g, 0.6f); CHECK_EQ(b, 0.9f);
}

TEST(conversion_stays_active_at_master_amount_zero_and_intensity_zero) {
    GradeSettings s;
    s.camera = 4;    // S-Log3 / S-Gamut3
    s.output = 3;    // sRGB
    s.look = 1;
    s.intensityPercent = 0;
    s.masterAmountPercent = 0;
    s.exposureStops = 2;
    float r = 420.0f / 1023, g = r, b = r;   // S-Log3 mid grey
    run3(s, r, g, b);
    CHECK_NEAR(r, encode_output(OutputTransfer::Srgb, 0.18), 1e-4);   // grey stays grey and is converted
    CHECK_NEAR(g, r, 1e-4);
    CHECK_NEAR(b, r, 1e-4);
    CHECK(!grade_is_noop(prepare_grade(s)));

    GradeSettings s2 = s;
    s2.masterAmountPercent = 100;   // exposure +2 EV now applies; look still off (intensity 0)
    float r2 = 420.0f / 1023, g2 = r2, b2 = r2;
    run3(s2, r2, g2, b2);
    CHECK_NEAR(r2, encode_output(OutputTransfer::Srgb, 0.72), 2e-4);
}

TEST(bypass_and_unavailable_profiles) {
    GradeSettings s;
    s.camera = 4; s.output = 3; s.bypass = true;
    CHECK(grade_is_noop(prepare_grade(s)));
    GradeSettings u;
    u.camera = 7;   // D-Log M
    const PreparedGrade pu = prepare_grade(u);
    CHECK(pu.unavailable && pu.bypass && grade_is_noop(pu));
    CHECK(std::string(profile_warning(u)).size() > 10);
    u.camera = 10;  // C-Log2
    CHECK(prepare_grade(u).unavailable);
    GradeSettings e;
    e.camera = 8;
    CHECK(std::string(profile_warning(e)).find("Experimental") != std::string::npos);
    CHECK_EQ(std::string(profile_warning(GradeSettings())), std::string(""));
}

TEST(basic_controls_behave) {
    auto lin = [](GradeSettings s) { s.camera = 1; s.output = 5; return s; };
    float r, g, b;
    // saturation 0 -> neutral grey with Rec.2020 luma
    GradeSettings s = lin(GradeSettings()); s.saturationPercent = 0;
    r = 0.6f; g = 0.3f; b = 0.1f; run3(s, r, g, b);
    CHECK_NEAR(r, g, 1e-6); CHECK_NEAR(g, b, 1e-6);
    CHECK_NEAR(r, 0.2627 * 0.6 + 0.678 * 0.3 + 0.0593 * 0.1, 1e-6);
    // contrast pivots at 0.18 and widens
    s = lin(GradeSettings()); s.contrast = 50;
    r = g = b = 0.18f; run3(s, r, g, b); CHECK_NEAR(r, 0.18, 1e-6);
    r = g = b = 0.36f; run3(s, r, g, b); CHECK(r > 0.36f);
    r = g = b = 0.09f; run3(s, r, g, b); CHECK(r < 0.09f);
    // warm temperature: more red than blue, luma preserved
    s = lin(GradeSettings()); s.temperature = 60;
    r = g = b = 0.18f; run3(s, r, g, b);
    CHECK(r > b); CHECK_NEAR(0.2627 * r + 0.678 * g + 0.0593 * b, 0.18, 1e-5);
    // shadows lift raises darks more than brights; highlights negative darkens brights more than darks
    s = lin(GradeSettings()); s.shadows = 60;
    float d = 0.02f, dg = 0.02f, db = 0.02f, h = 2.0f, hg = 2.0f, hb = 2.0f;
    run3(s, d, dg, db); run3(s, h, hg, hb);
    CHECK(d / 0.02f > 1.3f); CHECK(h / 2.0f < 1.05f);
    s = lin(GradeSettings()); s.highlights = -60;
    d = dg = db = 0.02f; h = hg = hb = 2.0f; run3(s, d, dg, db); run3(s, h, hg, hb);
    CHECK(h / 2.0f < 0.8f); CHECK(d / 0.02f > 0.97f);
}

TEST(basic_controls_are_monotonic_in_luma) {
    GradeSettings s; s.camera = 1; s.output = 5;
    s.shadows = 100; s.highlights = -100; s.contrast = 100; s.temperature = 40; s.tint = -30;
    const PreparedGrade p = prepare_grade(s);
    float prev = -1;
    for (int i = 1; i < 400; ++i) {
        const float v = 0.0005f * std::pow(1.02f, static_cast<float>(i));
        float r = v, g = v, b = v;
        grade_rgb(p, r, g, b);
        const float y = 0.2627f * r + 0.678f * g + 0.0593f * b;
        CHECK(y > prev); prev = y;
    }
}

TEST(everything_on_is_finite_for_hostile_input) {
    GradeSettings s;
    s.camera = 13; s.output = 4; s.look = 30; s.exposureStops = 4; s.contrast = 100; s.saturationPercent = 200;
    s.temperature = 100; s.tint = 100; s.highlights = 100; s.shadows = -100;
    const PreparedGrade p = prepare_grade(s);
    const float vals[] = {0.0f, -5.0f, 1e-30f, 1.0f, 17.0f, 1e20f, NAN, INFINITY, -INFINITY};
    for (float a : vals) for (float b2 : vals) {
        float r = a, g = b2, b = a;
        grade_rgb(p, r, g, b);
        CHECK(std::isfinite(r)); CHECK(std::isfinite(g)); CHECK(std::isfinite(b));
    }
}

TEST(look_intensity_blends_and_master_scales_only_grade) {
    GradeSettings s; s.camera = 1; s.output = 5; s.look = 3;
    s.intensityPercent = 100;
    float a[3] = {0.4f, 0.2f, 0.1f}, full[3];
    full[0] = a[0]; full[1] = a[1]; full[2] = a[2];
    run3(s, full[0], full[1], full[2]);
    s.intensityPercent = 50;
    float half[3] = {a[0], a[1], a[2]};
    run3(s, half[0], half[1], half[2]);
    for (int k = 0; k < 3; ++k) CHECK_NEAR(half[k], 0.5 * (a[k] + full[k]), 1e-5);
    s.intensityPercent = 100; s.masterAmountPercent = 50;
    float m[3] = {a[0], a[1], a[2]};
    run3(s, m[0], m[1], m[2]);
    for (int k = 0; k < 3; ++k) CHECK_NEAR(m[k], 0.5 * (a[k] + full[k]), 1e-5);
}
