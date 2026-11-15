#include "engine/camera/transfer.h"

#include <cmath>

namespace rwd {

namespace {
inline double p10(double x) { return std::pow(10.0, x); }

double slog3(double v) {
    const double c = v * 1023.0;
    if (c >= 171.2102946929) return p10((c - 420.0) / 261.5) * (0.18 + 0.01) - 0.01;
    return (c - 95.0) * 0.01125 / (171.2102946929 - 95.0);
}
double slog2(double v) {  // legal-range 10-bit code mapping (see CAMERA_PROFILES.md)
    const double y = (v * 1023.0 - 64.0) / 876.0;
    const double knee = 0.030001222851889303;
    const double x = (y >= knee) ? p10((y - 0.616596 - 0.03) / 0.432699) - 0.037584 : (y - knee) / 5.0;
    return x * 0.9 * 219.0 / 155.0;
}
double dlog(double v) {
    if (v <= 6.025 * 0.0078 + 0.0929) return (v - 0.0929) / 6.025;
    return (p10((v - 0.584555) / 0.256663) - 0.0108) / 0.9892;
}
double clog(double v) {  // Canon C-Log, scene reflection scaled by 0.9
    const double a = 0.529136, b = 10.1596, c = 0.0730597;
    if (v >= c) return (p10((v - c) / a) - 1.0) / b * 0.9;
    return -(p10((c - v) / a) - 1.0) / b * 0.9;
}
double clog3(double v) {
    const double lo = 1.9754798 * -0.014 + 0.12512219, hi = 1.9754798 * 0.014 + 0.12512219;
    double x;
    if (v < lo) x = -(p10((0.12783901 - v) / 0.36726845) - 1.0) / 14.98325;
    else if (v <= hi) x = (v - 0.12512219) / 1.9754798;
    else x = (p10((v - 0.12240537) / 0.36726845) - 1.0) / 14.98325;
    return x * 0.9;
}
double vlog(double v) {
    if (v < 0.181) return (v - 0.125) / 5.6;
    return p10((v - 0.598206) / 0.241514) - 0.00873;
}
double flog_generic(double v, double a, double b, double c, double d, double e, double f, double cut1) {
    const double cut2 = e * cut1 + f;
    if (v >= cut2) return (p10((v - d) / c) - b) / a;
    return (v - f) / e;
}
double nlog(double v) {
    const double c = v * 1023.0;
    if (c < 452.0) return std::pow(c / 650.0, 3.0) - 0.0075;
    return std::exp((c - 619.0) / 150.0);
}
double bmd5(double v) {
    const double a = 8.283605932402494, b = 0.09246575342465753, cut = 0.005;
    const double c = 0.08692876065491224, d = 0.005494072432257808, e = 0.5300133392291939;
    if (v < a * cut + b) return (v - b) / a;
    return std::exp((v - e) / c) - d;
}
}  // namespace

double decode_transfer(Transfer t, double v) {
    switch (t) {
        case Transfer::Gamma24: return std::copysign(std::pow(std::fabs(v), 2.4), v);
        case Transfer::Linear: return v;
        case Transfer::SLog2: return slog2(v);
        case Transfer::SLog3: return slog3(v);
        case Transfer::DLog: return dlog(v);
        case Transfer::CLog: return clog(v);
        case Transfer::CLog3: return clog3(v);
        case Transfer::VLog: return vlog(v);
        case Transfer::FLog: return flog_generic(v, 0.555556, 0.009468, 0.344676, 0.790453, 8.735631, 0.092864, 0.00089);
        case Transfer::FLog2: return flog_generic(v, 5.555556, 0.064829, 0.245281, 0.384316, 8.799461, 0.092864, 0.000889);
        case Transfer::NLog: return nlog(v);
        case Transfer::BmdFilmGen5: return bmd5(v);
    }
    return v;
}

}  // namespace rwd
