// RWD COLOR LAB - small 3x3 matrix helpers (double precision, used at prepare time only).
#pragma once
#include <cmath>

namespace rwd {

struct Mat3 { double m[9]; };  // row-major

struct Xy { double x, y; };

inline Mat3 mat_identity() { return Mat3{{1, 0, 0, 0, 1, 0, 0, 0, 1}}; }

inline Mat3 mat_mul(const Mat3& a, const Mat3& b) {
    Mat3 r{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            double s = 0;
            for (int k = 0; k < 3; ++k) s += a.m[i * 3 + k] * b.m[k * 3 + j];
            r.m[i * 3 + j] = s;
        }
    return r;
}

inline Mat3 mat_inverse(const Mat3& a) {  // callers only pass well-conditioned colour matrices
    const double* m = a.m;
    const double c00 = m[4] * m[8] - m[5] * m[7], c01 = m[5] * m[6] - m[3] * m[8], c02 = m[3] * m[7] - m[4] * m[6];
    const double det = m[0] * c00 + m[1] * c01 + m[2] * c02;
    const double id = 1.0 / det;
    return Mat3{{c00 * id, (m[2] * m[7] - m[1] * m[8]) * id, (m[1] * m[5] - m[2] * m[4]) * id,
                 c01 * id, (m[0] * m[8] - m[2] * m[6]) * id, (m[2] * m[3] - m[0] * m[5]) * id,
                 c02 * id, (m[1] * m[6] - m[0] * m[7]) * id, (m[0] * m[4] - m[1] * m[3]) * id}};
}

inline void mat_vec(const Mat3& a, const double v[3], double out[3]) {
    for (int i = 0; i < 3; ++i) out[i] = a.m[i * 3] * v[0] + a.m[i * 3 + 1] * v[1] + a.m[i * 3 + 2] * v[2];
}

// RGB -> XYZ from chromaticities (red, green, blue) and white point.
inline Mat3 rgb_to_xyz(const Xy p[3], Xy w) {
    Mat3 P{};
    for (int c = 0; c < 3; ++c) {
        P.m[0 * 3 + c] = p[c].x / p[c].y;
        P.m[1 * 3 + c] = 1.0;
        P.m[2 * 3 + c] = (1.0 - p[c].x - p[c].y) / p[c].y;
    }
    const double W[3] = {w.x / w.y, 1.0, (1.0 - w.x - w.y) / w.y};
    double S[3];
    mat_vec(mat_inverse(P), W, S);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c) P.m[r * 3 + c] *= S[c];
    return P;
}

// Bradford chromatic adaptation XYZ(src white) -> XYZ(dst white).
inline Mat3 bradford(Xy src, Xy dst) {
    const Mat3 B{{0.8951, 0.2664, -0.1614, -0.7502, 1.7135, 0.0367, 0.0389, -0.0685, 1.0296}};
    const double ws[3] = {src.x / src.y, 1.0, (1.0 - src.x - src.y) / src.y};
    const double wd[3] = {dst.x / dst.y, 1.0, (1.0 - dst.x - dst.y) / dst.y};
    double cs[3], cd[3];
    mat_vec(B, ws, cs);
    mat_vec(B, wd, cd);
    const Mat3 D{{cd[0] / cs[0], 0, 0, 0, cd[1] / cs[1], 0, 0, 0, cd[2] / cs[2]}};
    return mat_mul(mat_inverse(B), mat_mul(D, B));
}

}  // namespace rwd
