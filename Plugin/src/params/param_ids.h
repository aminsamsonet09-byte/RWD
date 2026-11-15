// RWD COLOR LAB - parameter identifiers.
//
// These strings are stored in saved Resolve projects. They are a compatibility
// contract: never rename or reuse one once a build has been released.
#pragma once

namespace rwd::param {

inline constexpr const char* kCamera = "rwd_camera";
inline constexpr const char* kOutput = "rwd_output";
inline constexpr const char* kStatus = "rwd_status";
inline constexpr const char* kLook = "rwd_look";
inline constexpr const char* kPrevLook = "rwd_prev_look";
inline constexpr const char* kNextLook = "rwd_next_look";
inline constexpr const char* kIntensity = "rwd_intensity";
inline constexpr const char* kExposure = "rwd_exposure";
inline constexpr const char* kContrast = "rwd_contrast";
inline constexpr const char* kSaturation = "rwd_saturation";
inline constexpr const char* kTemperature = "rwd_temperature";
inline constexpr const char* kTint = "rwd_tint";
inline constexpr const char* kHighlights = "rwd_highlights";
inline constexpr const char* kShadows = "rwd_shadows";
inline constexpr const char* kMasterAmount = "rwd_master_amount";
inline constexpr const char* kBypass = "rwd_bypass";
inline constexpr const char* kReset = "rwd_reset";

// Layout containers (not value parameters)
inline constexpr const char* kPageMain = "rwd_page_main";
inline constexpr const char* kGroupConversion = "rwd_group_conversion";
inline constexpr const char* kGroupLook = "rwd_group_look";
inline constexpr const char* kGroupBasic = "rwd_group_basic";
inline constexpr const char* kGroupGlobal = "rwd_group_global";

}  // namespace rwd::param
