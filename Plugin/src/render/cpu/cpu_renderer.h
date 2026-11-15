// RWD COLOR LAB - CPU reference renderer.
//
// This is the reliable baseline on every platform (Windows, macOS, Linux dev).
// GPU backends (Phase 7) must reproduce its results and are optional: nothing
// here depends on a GPU.
//
// Contract
//  - Works on RGBA images only; float32, uint16 and uint8 storage.
//  - Only pixels inside `window` (clipped to dst bounds) are written.
//  - Alpha is copied untouched, bit for bit. Only RGB is processed.
//  - Destination pixels not covered by the source image become 0,0,0,0.
//  - No allocation happens inside the pixel loops; the functions are
//    stateless, so disjoint windows may be rendered from several threads.
#pragma once

#include "engine/core/image_view.h"
#include "engine/grade_pipeline.h"

namespace rwd {

enum class RenderResult {
    Ok = 0,
    InvalidArguments,  // null/empty/overlapping-row buffer
    FormatMismatch     // src and dst pixel formats differ
};

const char* to_string(RenderResult r) noexcept;

// Optional cooperative-abort hook, polled once per row.
struct AbortCheck {
    bool (*fn)(void*) = nullptr;
    void* user = nullptr;
    bool operator()() const { return fn != nullptr && fn(user); }
};

// Grades `window` of `dst` from `src` (src may be null: output is cleared).
// When grade.bypass is set this performs an exact copy instead (copy_window).
RenderResult render_window(const ConstImageView* src, const ImageView& dst, const Bounds& window,
                           const PreparedGrade& grade, const AbortCheck& abort = AbortCheck());

// Exact byte copy of the overlap between src and `window`; the rest of the
// window is cleared. This is what Bypass uses.
RenderResult copy_window(const ConstImageView* src, const ImageView& dst, const Bounds& window,
                         const AbortCheck& abort = AbortCheck());

}  // namespace rwd
