// RWD COLOR LAB - camera conversion profile table. The index is stored in saved projects:
// only ever APPEND entries, never reorder.
#pragma once
#include "engine/camera/gamut.h"
#include "engine/camera/transfer.h"

namespace rwd {

enum class ProfileStatus {
    Verified,      // constants read from the manufacturer's own document, checked numerically
    Reference,     // matches a published reference implementation / check values; not read in the maker's doc here
    Experimental,  // part of the definition could not be confirmed; may be visibly inexact
    Unavailable    // no public specification found: the plug-in passes the image through unchanged
};

struct CameraProfile {
    const char* label;
    Transfer transfer;
    Gamut gamut;
    ProfileStatus status;
    const char* note;
};

constexpr int kCameraProfileCount = 16;
const CameraProfile& camera_profile(int index);   // out-of-range -> entry 0
int clamp_camera_index(int index);
bool camera_is_display_default(int index);        // true for entry 0

}  // namespace rwd
