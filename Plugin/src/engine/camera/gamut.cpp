#include "engine/camera/gamut.h"

namespace rwd {

namespace {
constexpr Xy kD65{0.3127, 0.3290};
constexpr Xy kD55{0.3324, 0.3474};

const GamutDef kDefs[] = {
    /* Rec709 */        {{{0.640, 0.330}, {0.300, 0.600}, {0.150, 0.060}}, kD65},
    /* Rec2020 */       {{{0.708, 0.292}, {0.170, 0.797}, {0.131, 0.046}}, kD65},
    /* SGamut */        {{{0.730, 0.280}, {0.140, 0.855}, {0.100, -0.050}}, kD65},
    /* SGamut3Cine */   {{{0.766, 0.275}, {0.225, 0.800}, {0.089, -0.087}}, kD65},
    /* DGamut */        {{{0.710, 0.310}, {0.210, 0.880}, {0.090, -0.080}}, kD65},
    /* VGamut */        {{{0.730, 0.280}, {0.165, 0.840}, {0.100, -0.030}}, {0.3127, 0.3290}},
    /* CinemaGamut */   {{{0.740, 0.270}, {0.170, 1.140}, {0.080, -0.100}}, kD55},
    /* BmdWideGamut5 */ {{{0.7177215, 0.3120576}, {0.2280410, 0.8615690}, {0.1005841, -0.0820452}},
                         {0.3127170, 0.3290312}},
};

Mat3 to_xyz_d65(Gamut g) {
    const GamutDef& d = gamut_def(g);
    Mat3 m = rgb_to_xyz(d.prim, d.white);
    if (std::fabs(d.white.x - kD65.x) > 1e-9 || std::fabs(d.white.y - kD65.y) > 1e-9)
        m = mat_mul(bradford(d.white, kD65), m);
    return m;
}
}  // namespace

const GamutDef& gamut_def(Gamut g) { return kDefs[static_cast<int>(g)]; }

Mat3 gamut_to_working(Gamut g) {
    return mat_mul(mat_inverse(to_xyz_d65(Gamut::Rec2020)), to_xyz_d65(g));
}
Mat3 working_to_gamut(Gamut g) {
    return mat_mul(mat_inverse(to_xyz_d65(g)), to_xyz_d65(Gamut::Rec2020));
}

}  // namespace rwd
