#include "engine/looks/look_recipes.h"

#include <algorithm>
#include <cmath>

namespace rwd {

namespace {
#define C "Cinematic"
#define K "Kodak-inspired"
#define F "Fujifilm-inspired"
#define V "Vintage"
#define M "Commercial / Fashion"
#define D "Mood"
#define Z {0, 0, 0}
const LookRecipe kLooks[kLookCount] = {
 {C,"Teal & Amber",       0,1.12f,1.05f,1.0f,1.0f,{-.25f,.04f,.22f},{.04f,0,-.04f},{.22f,.06f,-.28f},.004f,.4f,0},
 {C,"Blockbuster Punch",  0,1.25f,1.12f,1.1f,.9f, {-.1f,0,.1f},{.05f,0,-.03f},{.1f,.03f,-.1f},0,.6f,0},
 {C,"Noir Steel",        0,1.3f,.55f,.8f,.6f,    {-.05f,0,.12f},Z,{0,.02f,.06f},.006f,.5f,0},
 {C,"Golden Hour Epic",  .15f,1.1f,1.15f,1.0f,1.1f,{.02f,0,-.08f},{.1f,.04f,-.1f},{.2f,.08f,-.22f},.003f,.5f,0},
 {C,"Cold Thriller",    -.1f,1.2f,.85f,.9f,.7f,  {-.2f,0,.25f},{-.08f,0,.1f},{-.05f,0,.12f},.005f,.4f,0},
 {C,"Desert Dust",       .05f,1.08f,.9f,.9f,.95f,{.08f,0,-.12f},{.1f,.03f,-.12f},{.12f,.06f,-.1f},.01f,.3f,-4},
 {C,"Neon Night",       -.2f,1.2f,1.3f,1.2f,1.0f,{-.1f,-.1f,.3f},{.1f,-.1f,.1f},{.2f,-.05f,.1f},.002f,.4f,8},
 {C,"Muted Drama",       0,1.05f,.7f,.8f,.8f,    {-.05f,0,.05f},Z,{.03f,.01f,-.03f},.012f,.3f,0},
 {C,"Green Matrix",      0,1.15f,.9f,.9f,.85f,   {-.12f,.14f,-.1f},{-.06f,.1f,-.06f},{-.04f,.08f,-.04f},.004f,.3f,0},
 {C,"Soft Epic Fade",   .1f,.95f,.95f,1.0f,.95f, {.03f,.02f,.08f},Z,{.08f,.03f,-.04f},.02f,.5f,0},

 {K,"Amber Negative",    .1f,1.05f,1.1f,1.0f,1.05f,{.04f,0,-.06f},{.08f,.02f,-.08f},{.1f,.04f,-.1f},.008f,.35f,-3},
 {K,"Soft Portrait Film",.12f,.95f,.92f,1.0f,.9f,{.05f,.02f,0},{.06f,.03f,0},{.06f,.04f,.02f},.012f,.3f,-2},
 {K,"Rich Chrome Slide", 0,1.28f,1.25f,1.1f,1.1f,{-.03f,0,.05f},Z,{.05f,.02f,-.04f},0,.55f,2},
 {K,"Daylight Print",   .05f,1.1f,1.05f,1.0f,1.0f,{.02f,0,-.02f},{.03f,0,-.02f},{.03f,.01f,-.03f},.005f,.35f,0},
 {K,"Tungsten Cinema", -.05f,1.1f,.95f,.95f,.9f, {-.1f,0,.18f},{-.05f,0,.1f},{.02f,0,.05f},.006f,.4f,5},
 {K,"Golden Reversal",  .08f,1.2f,1.2f,1.05f,1.1f,{.06f,.02f,-.1f},{.12f,.05f,-.12f},{.15f,.06f,-.15f},.002f,.5f,-6},
 {K,"Heritage Colour",   0,1.0f,.88f,.95f,.85f,  {.03f,.03f,-.03f},{.05f,.04f,-.05f},{.07f,.05f,-.07f},.015f,.3f,-8},

 {F,"Cool Green Negative",.06f,1.02f,1.0f,.95f,.9f, {-.1f,.12f,.0f},{-.05f,.08f,-.02f},{-.02f,.05f,-.02f},.014f,.3f,16},
 {F,"Velvet Slide",   -.05f,1.3f,1.18f,1.15f,1.0f,{-.04f,-.02f,.08f},{-.02f,0,.03f},{.02f,0,0},0,.5f,-2},
 {F,"Clean Foliage",   .05f,1.08f,1.1f,1.0f,1.0f,{-.03f,.07f,0},{0,.06f,-.02f},{.02f,.05f,-.04f},.003f,.3f,10},
 {F,"Pastel Negative", .15f,.9f,.8f,.9f,.8f,     {.04f,.03f,.08f},{.05f,.04f,.05f},{.06f,.04f,.04f},.02f,.4f,0},
 {F,"Muted Documentary",-.1f,.98f,.58f,.8f,.7f,{-.06f,.03f,.07f},{-.02f,.02f,.02f},{.02f,.02f,-.01f},.018f,.3f,9},
 {F,"Teal Sky Chrome",  0,1.18f,1.0f,1.0f,.9f,   {-.12f,.04f,.14f},{-.04f,.02f,.06f},{.04f,0,-.06f},.004f,.45f,-9},

 {V,"Faded 70s",       .1f,.88f,.85f,.9f,.8f,    {.1f,.05f,-.02f},{.08f,.05f,-.06f},{.1f,.06f,-.1f},.035f,.3f,-10},
 {V,"Sepia Archive",    0,1.0f,.25f,.5f,.4f,     {.1f,.04f,-.12f},{.15f,.06f,-.18f},{.2f,.08f,-.22f},.025f,.3f,0},
 {V,"Super 8 Warm",    .05f,1.12f,1.1f,1.0f,1.0f,{.05f,0,-.08f},{.1f,.03f,-.1f},{.2f,.1f,-.12f},.02f,.5f,-5},
 {V,"Instant Wash",    .2f,.85f,.75f,.85f,.7f,   {.04f,.06f,.1f},{.04f,.05f,.04f},{.08f,.05f,-.02f},.045f,.25f,12},
 {V,"Cross-Process Retro",0,1.3f,1.3f,1.2f,1.2f, {-.2f,.1f,.2f},{.05f,.12f,-.12f},{.25f,.1f,-.3f},0,.4f,18},
 {V,"Old Newsroom",  -.05f,1.1f,.45f,.6f,.5f,    {.02f,.04f,0},{.04f,.05f,-.04f},{.06f,.06f,-.06f},.03f,.3f,0},

 {M,"Clean Commercial", .1f,1.08f,1.1f,1.05f,1.05f,Z,Z,Z,0,.4f,0},
 {M,"High-Key Beauty",  .3f,.92f,.95f,.9f,.85f,  {.03f,.01f,.01f},{.03f,.02f,.02f},{.02f,.02f,.02f},.008f,.6f,0},
 {M,"Bright Lifestyle", .2f,1.05f,1.2f,1.1f,1.1f,{0,.02f,.04f},{.02f,.02f,0},{.05f,.03f,-.03f},0,.5f,3},
 {M,"Magazine Punch",  .05f,1.3f,1.28f,1.2f,1.15f,{-.03f,0,.04f},Z,{.03f,.01f,-.02f},0,.6f,0},
 {M,"Skin Glow",       .15f,.98f,1.0f,.95f,.9f,  {.05f,.02f,0},{.06f,.03f,.01f},{.06f,.04f,.02f},.006f,.5f,-4},
 {M,"Pastel Fashion",  .2f,.92f,.9f,.95f,.85f,   {.04f,0,.06f},{.05f,.03f,.05f},{.05f,.04f,.05f},.01f,.5f,-14},
 {M,"Crisp Product",   .05f,1.2f,1.0f,1.0f,1.0f, {-.02f,-.02f,-.02f},Z,{.02f,.02f,.02f},0,.35f,0},
 {M,"Urban Streetwear",-.05f,1.25f,.95f,.9f,.9f, {-.1f,-.04f,.08f},Z,{.08f,.04f,-.04f},.006f,.45f,-3},

 {D,"Melancholy Blue", -.15f,1.05f,.75f,.85f,.7f,{-.12f,-.02f,.22f},{-.08f,-.02f,.15f},{-.04f,0,.1f},.012f,.35f,0},
 {D,"Warm Nostalgia",   .05f,.98f,.95f,1.0f,.95f,{.08f,.03f,-.03f},{.1f,.05f,-.06f},{.14f,.08f,-.1f},.015f,.4f,-7},
 {D,"Dreamy Haze",     .2f,.82f,.85f,.9f,.75f,   {.04f,.04f,.1f},{.06f,.05f,.07f},{.08f,.06f,.06f},.05f,.7f,4},
 {D,"Dark Moody",     -.35f,1.3f,.8f,.85f,.7f,   {-.06f,-.02f,.1f},{-.02f,0,.04f},{.02f,0,-.02f},.004f,.5f,0},
 {D,"Romantic Rose",   .1f,.95f,1.0f,1.0f,.9f,   {.1f,-.04f,.04f},{.1f,-.02f,.04f},{.12f,0,.02f},.012f,.4f,-16},
 {D,"Autumn Calm",      0,1.05f,.9f,.95f,.9f,    {.06f,.02f,-.08f},{.1f,.04f,-.1f},{.12f,.06f,-.12f},.01f,.35f,-12},
 {D,"Winter Frost",   .05f,1.0f,.7f,.8f,.7f,     {-.08f,0,.14f},{-.06f,0,.1f},{-.04f,.02f,.1f},.01f,.4f,14},
 {D,"Sunset Fire",      0,1.2f,1.3f,1.1f,1.2f,   {.15f,-.05f,-.12f},{.2f,0,-.2f},{.3f,.1f,-.3f},.002f,.5f,-10},
 {D,"Mono Silver",      0,1.2f,0,1.0f,1.0f,      {-.02f,0,.04f},Z,Z,.008f,.5f,0},
 {D,"Mono Warm",        0,1.1f,0,1.0f,1.0f,      {.1f,.04f,-.08f},{.1f,.05f,-.08f},{.14f,.07f,-.1f},.015f,.4f,0},
};
#undef C
#undef K
#undef F
#undef V
#undef M
#undef D
#undef Z

constexpr float kLw[3] = {0.2627f, 0.6780f, 0.0593f};
inline float luma(const float* c) { return kLw[0] * c[0] + kLw[1] * c[1] + kLw[2] * c[2]; }
}  // namespace

const LookRecipe& look_recipe(int i) { return kLooks[(i < 0 || i >= kLookCount) ? 0 : i]; }

const char* collection_name(int c) {
    static const char* n[kCollectionCount] = {"Cinematic", "Kodak-inspired", "Fujifilm-inspired", "Vintage", "Commercial / Fashion", "Mood"};
    return n[(c < 0 || c >= kCollectionCount) ? 0 : c];
}

void apply_recipe(const LookRecipe& r, float c[3]) {
    const float eg = std::exp2(r.exposure);
    for (int i = 0; i < 3; ++i) c[i] *= eg;

    // contrast about 0.18 on luma ratio (hue preserving)
    float Y = luma(c);
    if (Y > 1e-6f && r.contrast != 1.0f) {
        const float k = 0.18f * std::pow(Y / 0.18f, r.contrast) / Y;
        for (int i = 0; i < 3; ++i) c[i] *= k;
        Y = luma(c);
    }
    // highlight shoulder
    if (Y > 0.5f && r.shoulder > 0) {
        const float d = Y - 0.5f;
        const float k = (0.5f + d / (1.0f + r.shoulder * d)) / Y;
        for (int i = 0; i < 3; ++i) c[i] *= k;
    }
    // hue rotation about the grey axis (Rodrigues)
    if (r.hueDegrees != 0) {
        const float t = r.hueDegrees * 0.01745329252f, co = std::cos(t), si = std::sin(t);
        const float a = (1 - co) / 3.0f, b = si * 0.57735026919f;
        const float R = c[0], G = c[1], B = c[2];
        c[0] = (co + a) * R + (a - b) * G + (a + b) * B;
        c[1] = (a + b) * R + (co + a) * G + (a - b) * B;
        c[2] = (a - b) * R + (a + b) * G + (co + a) * B;
    }
    // masks in stops about mid grey
    Y = luma(c);
    const float s = std::log2(std::max(Y, 1e-6f) / 0.18f);
    const float ms = 1.0f / (1.0f + std::exp(1.5f * (s + 1.5f)));
    const float mh = 1.0f / (1.0f + std::exp(-1.5f * (s - 1.5f)));
    float mm = 1.0f - ms - mh;
    if (mm < 0) mm = 0;
    // saturation
    const float se = r.saturation * (1.0f + (r.satShadows - 1.0f) * ms + (r.satHighlights - 1.0f) * mh);
    for (int i = 0; i < 3; ++i) c[i] = Y + (c[i] - Y) * se;
    // tints
    for (int i = 0; i < 3; ++i) c[i] *= std::exp2(ms * r.shadowTint[i] + mm * r.midTint[i] + mh * r.highTint[i]);
    // faded blacks
    for (int i = 0; i < 3; ++i) c[i] = r.fade + c[i] * (1.0f - r.fade);
}

}  // namespace rwd
