#include "render/cpu/cpu_renderer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace rwd {

const char* to_string(RenderResult r) noexcept {
    switch (r) {
        case RenderResult::Ok: return "ok";
        case RenderResult::InvalidArguments: return "invalid image arguments";
        case RenderResult::FormatMismatch: return "source and destination pixel formats differ";
    }
    return "unknown";
}

namespace {

// ---- pixel format traits: storage <-> normalised float ---------------------
template <typename T> struct PixelTraits;

template <> struct PixelTraits<float> {
    static float to_float(float v) noexcept { return v; }
    static float from_float(float v) noexcept { return v; }
};

template <> struct PixelTraits<std::uint16_t> {
    static float to_float(std::uint16_t v) noexcept { return static_cast<float>(v) * (1.0f / 65535.0f); }
    static std::uint16_t from_float(float v) noexcept {
        return static_cast<std::uint16_t>(clamp01(v) * 65535.0f + 0.5f);
    }
};

template <> struct PixelTraits<std::uint8_t> {
    static float to_float(std::uint8_t v) noexcept { return static_cast<float>(v) * (1.0f / 255.0f); }
    static std::uint8_t from_float(float v) noexcept {
        return static_cast<std::uint8_t>(clamp01(v) * 255.0f + 0.5f);
    }
};

RenderResult validate(const ConstImageView* src, const ImageView& dst) noexcept {
    if (!dst.usable()) return RenderResult::InvalidArguments;
    if (src != nullptr) {
        if (!src->usable()) return RenderResult::InvalidArguments;
        if (src->format != dst.format) return RenderResult::FormatMismatch;
    }
    return RenderResult::Ok;
}

inline std::ptrdiff_t byte_offset(int pixels, std::size_t bpp) noexcept {
    return static_cast<std::ptrdiff_t>(pixels) * static_cast<std::ptrdiff_t>(bpp);
}

// ---- span kernels ------------------------------------------------------------
// Pixels are moved with memcpy so no alignment or aliasing assumption is made
// about host buffers; compilers turn these into plain loads/stores.
template <typename T>
void grade_span(const unsigned char* s, unsigned char* d, int count, const PreparedGrade& grade) noexcept {
    using Tr = PixelTraits<T>;
    constexpr std::size_t kBpp = sizeof(T) * kChannels;
    for (int i = 0; i < count; ++i) {
        T in[4];
        std::memcpy(in, s, sizeof in);
        float r = Tr::to_float(in[0]);
        float g = Tr::to_float(in[1]);
        float b = Tr::to_float(in[2]);
        grade_rgb(grade, r, g, b);
        const T out[4] = {Tr::from_float(r), Tr::from_float(g), Tr::from_float(b), in[3]};  // alpha untouched
        std::memcpy(d, out, sizeof out);
        s += kBpp;
        d += kBpp;
    }
}

// Shared row/column walk. `Span(src_ptr_or_null, dst_ptr, count)` handles the
// part of a row that has a source; everything else is cleared.
template <typename SpanFn>
void walk_window(const ConstImageView* src, const ImageView& dst, const Bounds& w, std::size_t bpp,
                 const AbortCheck& abort, SpanFn&& span) {
    for (int y = w.y1; y < w.y2; ++y) {
        if (abort()) break;

        unsigned char* dstRow = dst.row(y) + byte_offset(w.x1 - dst.bounds.x1, bpp);
        const std::size_t fullBytes = static_cast<std::size_t>(w.x2 - w.x1) * bpp;

        // x-range of this row that is covered by the source image
        int ox1 = w.x1, ox2 = w.x1;  // empty by default
        if (src != nullptr && y >= src->bounds.y1 && y < src->bounds.y2) {
            const int a = std::max(w.x1, src->bounds.x1);
            const int b = std::min(w.x2, src->bounds.x2);
            if (a < b) { ox1 = a; ox2 = b; }
        }

        if (ox1 >= ox2) {  // no source for this row: clear it
            std::memset(dstRow, 0, fullBytes);
            continue;
        }

        const std::size_t leftBytes = static_cast<std::size_t>(ox1 - w.x1) * bpp;
        const std::size_t rightBytes = static_cast<std::size_t>(w.x2 - ox2) * bpp;
        if (leftBytes) std::memset(dstRow, 0, leftBytes);

        const unsigned char* srcPix = src->row(y) + byte_offset(ox1 - src->bounds.x1, bpp);
        span(srcPix, dstRow + leftBytes, ox2 - ox1);

        if (rightBytes) std::memset(dstRow + leftBytes + static_cast<std::size_t>(ox2 - ox1) * bpp, 0, rightBytes);
    }
}

template <typename T>
void grade_window_t(const ConstImageView* src, const ImageView& dst, const Bounds& w,
                    const PreparedGrade& grade, const AbortCheck& abort) {
    constexpr std::size_t kBpp = sizeof(T) * kChannels;
    walk_window(src, dst, w, kBpp, abort,
                [&](const unsigned char* s, unsigned char* d, int count) { grade_span<T>(s, d, count, grade); });
}

}  // namespace

RenderResult copy_window(const ConstImageView* src, const ImageView& dst, const Bounds& window,
                         const AbortCheck& abort) {
    const RenderResult v = validate(src, dst);
    if (v != RenderResult::Ok) return v;

    const Bounds w = intersect(window, dst.bounds);
    if (w.empty()) return RenderResult::Ok;

    const std::size_t bpp = bytes_per_pixel(dst.format);
    walk_window(src, dst, w, bpp, abort, [&](const unsigned char* s, unsigned char* d, int count) {
        // memmove: tolerate a host that passes the same buffer as src and dst
        std::memmove(d, s, static_cast<std::size_t>(count) * bpp);
    });
    return RenderResult::Ok;
}

RenderResult render_window(const ConstImageView* src, const ImageView& dst, const Bounds& window,
                           const PreparedGrade& grade, const AbortCheck& abort) {
    if (grade.bypass) return copy_window(src, dst, window, abort);

    const RenderResult v = validate(src, dst);
    if (v != RenderResult::Ok) return v;

    const Bounds w = intersect(window, dst.bounds);
    if (w.empty()) return RenderResult::Ok;

    switch (dst.format) {
        case PixelFormat::Float32: grade_window_t<float>(src, dst, w, grade, abort); break;
        case PixelFormat::UInt16: grade_window_t<std::uint16_t>(src, dst, w, grade, abort); break;
        case PixelFormat::UInt8: grade_window_t<std::uint8_t>(src, dst, w, grade, abort); break;
    }
    return RenderResult::Ok;
}

}  // namespace rwd
