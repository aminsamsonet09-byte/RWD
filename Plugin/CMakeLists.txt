# ===========================================================================
# 1) rwd_engine : platform-independent CPU reference engine (no OpenFX).
# ===========================================================================
add_library(rwd_engine STATIC
  src/engine/core/numeric.h
  src/engine/core/image_view.h
  src/engine/working/exposure.h
  src/engine/grade_pipeline.h
  src/engine/grade_pipeline.cpp
  src/engine/color/mat3.h
  src/engine/camera/gamut.h
  src/engine/camera/gamut.cpp
  src/engine/camera/transfer.h
  src/engine/camera/transfer.cpp
  src/engine/camera/camera_profiles.h
  src/engine/camera/camera_profiles.cpp
  src/engine/output/output_profiles.h
  src/engine/output/output_profiles.cpp
  src/engine/looks/look_recipes.h
  src/engine/looks/look_recipes.cpp
  src/engine/looks/look_lut.h
  src/engine/looks/look_lut.cpp
  src/engine/working/grade_ops.h
  src/render/cpu/cpu_renderer.h
  src/render/cpu/cpu_renderer.cpp)
target_include_directories(rwd_engine PUBLIC src)
target_compile_features(rwd_engine PUBLIC cxx_std_17)
# Engine symbols stay private inside the plug-in binary (only OFX entry points are exported).
set_target_properties(rwd_engine PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(rwd_engine PRIVATE rwd_warnings)

if(NOT RWD_BUILD_PLUGIN)
  return()
endif()

# ===========================================================================
# 2) ofx_support : official OpenFX 1.5 C++ Support library (vendored, BSD-3).
#    Source: https://github.com/AcademySoftwareFoundation/openfx
#    Tag OFX_Release_1.5, commit a98b77b0fd61b8dafb82f20e93d9b82ac97a55a5
#    (see third_party/THIRD_PARTY.md)
# ===========================================================================
set(OFX_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openfx")
file(GLOB OFX_SUPPORT_SOURCES CONFIGURE_DEPENDS "${OFX_SDK_DIR}/Support/Library/*.cpp")
add_library(ofx_support STATIC ${OFX_SUPPORT_SOURCES})
# SYSTEM: vendored headers must not produce warnings in RWD code.
target_include_directories(ofx_support SYSTEM PUBLIC
  "${OFX_SDK_DIR}/include"
  "${OFX_SDK_DIR}/Support/include")
target_compile_features(ofx_support PUBLIC cxx_std_17)
# Keep the vendored C++ symbols private: the .ofx must export only the OFX entry
# points (OfxGetNumberOfPlugins / OfxGetPlugin) so it cannot clash with other
# OFX plug-ins loaded into the same Resolve process.
set_target_properties(ofx_support PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)
if(MSVC)
  target_compile_definitions(ofx_support PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(ofx_support PRIVATE /W3 /utf-8)
else()
  target_compile_options(ofx_support PRIVATE -w)
endif()

# ===========================================================================
# 3) RWD_COLOR_LAB.ofx : the plug-in itself.
# ===========================================================================
set(RWD_PLUGIN_NAME RWD_COLOR_LAB)
set(RWD_PLUGIN_SOURCES
  src/host/plugin_info.h
  src/host/plugin_factory.cpp
  src/host/rwd_effect.h
  src/host/rwd_effect.cpp
  src/params/param_ids.h
  src/params/param_definitions.h
  src/params/param_definitions.cpp
  src/params/param_reader.h
  src/params/param_reader.cpp)

if(APPLE)
  add_library(${RWD_PLUGIN_NAME} MODULE ${RWD_PLUGIN_SOURCES})  # Mach-O bundle
else()
  add_library(${RWD_PLUGIN_NAME} SHARED ${RWD_PLUGIN_SOURCES})  # DLL / .so
endif()
set_target_properties(${RWD_PLUGIN_NAME} PROPERTIES
  PREFIX ""
  SUFFIX ".ofx"
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(${RWD_PLUGIN_NAME} PRIVATE src)
target_link_libraries(${RWD_PLUGIN_NAME} PRIVATE rwd_engine ofx_support rwd_warnings)
target_compile_definitions(${RWD_PLUGIN_NAME} PRIVATE
  RWD_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  RWD_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  RWD_VERSION_PATCH=${PROJECT_VERSION_PATCH})

# --- OFX bundle layout --------------------------------------------------------
#   RWD_COLOR_LAB.ofx.bundle/Contents/{Info.plist (macOS), <ARCH>/RWD_COLOR_LAB.ofx}
if(APPLE)
  set(RWD_ARCH_DIR "MacOS")
elseif(WIN32)
  set(RWD_ARCH_DIR "Win64")
  if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only 64-bit Windows (x64) builds are supported")
  endif()
elseif(UNIX)
  set(RWD_ARCH_DIR "Linux-x86-64")  # development only
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

set(RWD_BUNDLE_NAME "${RWD_PLUGIN_NAME}.ofx.bundle")
set(RWD_BUNDLE_DIR "${CMAKE_BINARY_DIR}/bundle/$<CONFIG>/${RWD_BUNDLE_NAME}")

add_custom_command(TARGET ${RWD_PLUGIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${RWD_BUNDLE_DIR}/Contents/${RWD_ARCH_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:${RWD_PLUGIN_NAME}>"
          "${RWD_BUNDLE_DIR}/Contents/${RWD_ARCH_DIR}/${RWD_PLUGIN_NAME}.ofx"
  VERBATIM)

if(APPLE)
  set(RWD_BUNDLE_IDENTIFIER "id.ridwanard.rwdcolorlab")
  set(RWD_BUNDLE_VERSION "${PROJECT_VERSION}")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/Info.plist.in"
                 "${CMAKE_BINARY_DIR}/Info.plist" @ONLY)
  add_custom_command(TARGET ${RWD_PLUGIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/Info.plist" "${RWD_BUNDLE_DIR}/Contents/Info.plist"
    VERBATIM)

  if(RWD_MACOS_CODESIGN_IDENTITY)
    # Ad-hoc ("-") signing is enough to load locally; distributing to other Macs
    # needs a Developer ID identity (and notarization, which is NOT automated here).
    if(RWD_MACOS_CODESIGN_IDENTITY STREQUAL "-")
      set(RWD_CODESIGN_ARGS --force --sign "-")
    else()
      set(RWD_CODESIGN_ARGS --force --timestamp --sign "${RWD_MACOS_CODESIGN_IDENTITY}")
    endif()
    add_custom_command(TARGET ${RWD_PLUGIN_NAME} POST_BUILD
      COMMAND codesign ${RWD_CODESIGN_ARGS} "${RWD_BUNDLE_DIR}"
      VERBATIM)
  endif()
endif()

# --- install -------------------------------------------------------------------
if(WIN32)
  set(RWD_DEFAULT_INSTALL_DIR "C:/Program Files/Common Files/OFX/Plugins")
elseif(APPLE)
  set(RWD_DEFAULT_INSTALL_DIR "/Library/OFX/Plugins")
else()
  set(RWD_DEFAULT_INSTALL_DIR "/usr/OFX/Plugins")
endif()
set(RWD_PLUGIN_INSTALL_DIR "${RWD_DEFAULT_INSTALL_DIR}" CACHE PATH
    "Where 'cmake --install' puts the .ofx.bundle (needs administrator rights for the defaults)")
install(DIRECTORY "${RWD_BUNDLE_DIR}"
        DESTINATION "${RWD_PLUGIN_INSTALL_DIR}"
        USE_SOURCE_PERMISSIONS)
