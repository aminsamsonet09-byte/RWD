// RWD COLOR LAB - plug-in identity.
#pragma once

#ifndef RWD_VERSION_MAJOR
#define RWD_VERSION_MAJOR 0
#endif
#ifndef RWD_VERSION_MINOR
#define RWD_VERSION_MINOR 3
#endif
#ifndef RWD_VERSION_PATCH
#define RWD_VERSION_PATCH 0
#endif

namespace rwd {

// Reverse-DNS identifier. Resolve stores this inside saved projects/grades:
// NEVER change it after a public release or existing projects lose the effect.
inline constexpr const char* kPluginIdentifier = "id.ridwanard.RWDColorLab";
inline constexpr const char* kPluginLabel = "RWD COLOR LAB";
inline constexpr const char* kPluginGrouping = "RWD";
inline constexpr const char* kPluginDescription =
    "RWD COLOR LAB by ridwanard.id - Professional Color Transformation & Creative Looks.\n"
    "Phase 2 prototype: Exposure, Master Amount and Bypass on the CPU.";

// OFX plug-in version. The minor number is bumped for each release; the major
// number changes only when saved parameter data becomes incompatible.
inline constexpr unsigned kPluginVersionMajor = RWD_VERSION_MAJOR;
inline constexpr unsigned kPluginVersionMinor = RWD_VERSION_MINOR;

}  // namespace rwd
