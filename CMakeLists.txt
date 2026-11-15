cmake_minimum_required(VERSION 3.21)

# ---------------------------------------------------------------------------
# RWD COLOR LAB - one shared C++ code base, one product, per-OS OFX bundles.
#   Windows  : MSVC (Visual Studio 2022), x64      -> Contents/Win64
#   macOS    : Apple Silicon + Intel (universal)   -> Contents/MacOS
#   Linux    : development / unit-test platform only (not a product target)
# ---------------------------------------------------------------------------

# --- settings that must be decided before project() -------------------------
if(APPLE)
  if(NOT CMAKE_OSX_ARCHITECTURES)
    # "arm64;x86_64" = universal binary (preferred by the OpenFX 1.5 packaging
    # rules). Use -DCMAKE_OSX_ARCHITECTURES=arm64 for an Apple-Silicon-only build.
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures")
  endif()
  if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    # 11.0 is the first macOS release that supports arm64.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
  endif()
endif()

option(RWD_STATIC_MSVC_RUNTIME "Windows: link the MSVC runtime statically (no VC++ Redistributable needed)" ON)
if(MSVC AND RWD_STATIC_MSVC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(RWDColorLab VERSION 0.3.0 LANGUAGES CXX)

# --- options -----------------------------------------------------------------
option(RWD_BUILD_PLUGIN "Build the OpenFX plug-in bundle" ON)
option(RWD_BUILD_TESTS "Build the unit tests" ON)
option(RWD_WARNINGS_AS_ERRORS "Treat warnings in RWD sources as errors" OFF)
option(RWD_SANITIZE "GCC/Clang: build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
set(RWD_MACOS_CODESIGN_IDENTITY "-" CACHE STRING
    "macOS codesign identity for the bundle ('-' = ad-hoc, empty = skip signing)")

# --- global compile settings -------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_property(RWD_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT RWD_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(RWD_SANITIZE)
  if(MSVC)
    message(FATAL_ERROR "RWD_SANITIZE is only supported with GCC/Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# Warning flags for RWD's own sources (vendored OpenFX code is not affected).
# NOTE: never enable fast-math: the engine relies on IEEE NaN/Inf semantics.
add_library(rwd_warnings INTERFACE)
if(MSVC)
  target_compile_options(rwd_warnings INTERFACE /W4 /utf-8 /fp:precise)
  if(RWD_WARNINGS_AS_ERRORS)
    target_compile_options(rwd_warnings INTERFACE /WX)
  endif()
else()
  target_compile_options(rwd_warnings INTERFACE -Wall -Wextra -Wpedantic)
  if(RWD_WARNINGS_AS_ERRORS)
    target_compile_options(rwd_warnings INTERFACE -Werror)
  endif()
endif()

add_subdirectory(Plugin)

if(RWD_BUILD_TESTS)
  enable_testing()
  add_subdirectory(Tests)
endif()
