// RWD COLOR LAB - colour gamut definitions (chromaticities) and conversion to the working space.
// Working space: linear Rec.2020 primaries, D65 white. Matrices are DERIVED from the primaries
// (never typed in), with a Bradford adaptation only when the source white is not D65.
#pragma once
#include "engine/color/mat3.h"

namespace rwd {

enum class Gamut {
    Rec709,       // ITU-R BT.709 / sRGB primaries, D65
    Rec2020,      // ITU-R BT.2020, D65 (also Nikon N-Gamut and Fujifilm F-Gamut per their documents)
    SGamut,       // Sony S-Gamut / S-Gamut3 (identical primaries), D65
    SGamut3Cine,  // Sony S-Gamut3.Cine, D65
    DGamut,       // DJI D-Gamut, D65 (matrix cross-checked with the DJI whitepaper)
    VGamut,       // Panasonic V-Gamut, D65 (matrix cross-checked with the Panasonic document)
    CinemaGamut,  // Canon Cinema Gamut - EXPERIMENTAL (white point not confirmed; D55 assumed)
    BmdWideGamut5 // Blackmagic Wide Gamut Gen 5 - EXPERIMENTAL primaries (not cross-checked)
};

struct GamutDef { Xy prim[3]; Xy white; };
const GamutDef& gamut_def(Gamut g);

Mat3 gamut_to_working(Gamut g);   // gamut-linear RGB -> working-space RGB
Mat3 working_to_gamut(Gamut g);   // working-space RGB -> gamut-linear RGB

}  // namespace rwd
