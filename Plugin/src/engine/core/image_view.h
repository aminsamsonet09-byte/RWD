// RWD COLOR LAB - lightweight, non-owning image descriptions used by the CPU
// reference engine. They mirror the OpenFX image model (bounds, signed row
// bytes, pointer to the pixel at (bounds.x1, bounds.y1)) without depending on it,
// so the engine can be unit-tested on any platform.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace rwd {

enum class PixelFormat { Float32, UInt16, UInt8 };

inline constexpr std::size_t kChannels = 4;  // RGBA, always

inline constexpr std::size_t bytes_per_channel(PixelFormat f) noexcept {
    return f == PixelFormat::Float32 ? 4u : (f == PixelFormat::UInt16 ? 2u : 1u);
}
inline constexpr std::size_t bytes_per_pixel(PixelFormat f) noexcept {
    return bytes_per_channel(f) * kChannels;
}

// Half-open pixel rectangle [x1,x2) x [y1,y2) in OpenFX pixel coordinates.
struct Bounds {
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    bool empty() const noexcept { return x1 >= x2 || y1 >= y2; }
    int width() const noexcept { return x2 - x1; }
    int height() const noexcept { return y2 - y1; }
};

inline Bounds intersect(const Bounds& a, const Bounds& b) noexcept {
    Bounds r;
    r.x1 = std::max(a.x1, b.x1);
    r.y1 = std::max(a.y1, b.y1);
    r.x2 = std::min(a.x2, b.x2);
    r.y2 = std::min(a.y2, b.y2);
    return r;
}

// True when the described buffer can be addressed safely: non-null, non-empty,
// and rows do not overlap each other (|rowBytes| >= one row of pixels).
inline bool layout_usable(bool hasData, const Bounds& bounds, std::ptrdiff_t rowBytes,
                          PixelFormat format) noexcept {
    if (!hasData || bounds.empty()) return false;
    const std::int64_t rowSpan =
        static_cast<std::int64_t>(bounds.width()) * static_cast<std::int64_t>(bytes_per_pixel(format));
    const std::int64_t absStride = rowBytes < 0 ? -static_cast<std::int64_t>(rowBytes)
                                                : static_cast<std::int64_t>(rowBytes);
    if (bounds.height() > 1 && absStride < rowSpan) return false;
    return true;
}

// Read-only image. `data` points at the pixel (bounds.x1, bounds.y1);
// `rowBytes` may be negative (bottom-up storage) or include padding.
struct ConstImageView {
    const unsigned char* data = nullptr;
    Bounds bounds{};
    std::ptrdiff_t rowBytes = 0;
    PixelFormat format = PixelFormat::Float32;

    bool usable() const noexcept { return layout_usable(data != nullptr, bounds, rowBytes, format); }
    // Address of pixel (bounds.x1, y). Caller guarantees y is inside bounds.
    const unsigned char* row(int y) const noexcept {
        return data + static_cast<std::ptrdiff_t>(y - bounds.y1) * rowBytes;
    }
};

// Writable image, same conventions as ConstImageView.
struct ImageView {
    unsigned char* data = nullptr;
    Bounds bounds{};
    std::ptrdiff_t rowBytes = 0;
    PixelFormat format = PixelFormat::Float32;

    bool usable() const noexcept { return layout_usable(data != nullptr, bounds, rowBytes, format); }
    unsigned char* row(int y) const noexcept {
        return data + static_cast<std::ptrdiff_t>(y - bounds.y1) * rowBytes;
    }
};

}  // namespace rwd
