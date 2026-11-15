# RWD COLOR LAB

**By ridwanard.id** — Professional Color Transformation & Creative Looks
Native OpenFX plug-in for DaVinci Resolve. One shared C++17 code base; separate native bundles for Windows and macOS.

> **Status: v0.3.0 — feature-complete, build-ready, NOT yet verified in DaVinci Resolve.**
> Compiled and unit-tested on Linux only (GCC/Clang, 44 tests). Windows/macOS builds come from the included GitHub Actions workflow and have not been run yet.

## What you get in the Inspector

| Group | Controls |
|---|---|
| Conversion | Conversion LUT (input camera profile), Conversion output, Profile status |
| Creative LUT | Creative LUT dropdown (None + 47 looks), Prev LUT, Next LUT, Intensity 0–100 % |
| Basic Adjustments | Exposure ±4 EV, Contrast, Saturation 0–200 %, Temperature, Tint, Highlights, Shadows |
| Global Controls | Master Amount 0–100 %, Bypass, Reset |

Resolve draws OFX parameters itself: standard Inspector sliders/dropdowns, no custom dark UI or LUT thumbnails.

Details: [CAMERA_PROFILES](Documentation/CAMERA_PROFILES.md) · [COLOR_SCIENCE](Documentation/COLOR_SCIENCE.md) · [CREATIVE_LOOKS](Documentation/CREATIVE_LOOKS.md) · [TEST_REPORT](Documentation/TEST_REPORT.md) · [COMPATIBILITY](Documentation/COMPATIBILITY.md)

## Get the installable bundles
Push this folder to GitHub → Actions → *build* → download `RWD_COLOR_LAB_Windows_x64` / `RWD_COLOR_LAB_macOS_universal` → follow [INSTALLATION_GUIDE](Documentation/INSTALLATION_GUIDE.md).

## Project structure

```
RWD_COLOR_LAB/
  CMakeLists.txt, CMakePresets.json
  Plugin/
    src/engine/      platform-independent colour engine (no OpenFX): numeric helpers,
                     image views, exposure, grade pipeline
    src/render/cpu/  CPU reference renderer (the baseline on every platform)
    src/params/      OFX parameter ids, definitions, value reader
    src/host/        OFX plug-in factory (describe / describeInContext) and effect instance
    packaging/       macOS Info.plist template
    third_party/     vendored official OpenFX 1.5 SDK (BSD-3), see THIRD_PARTY.md
  Tests/
    unit/            engine unit tests (no OpenFX, no Resolve)
    plugin_load/     loads the built .ofx and checks its OFX exports/descriptor
  Tools/             build_windows.ps1, build_macos.sh
  Resources/         placeholders for camera / output profiles and Creative LUTs (empty)
  Documentation/     architecture and guides
  Builds/            build output (git-ignored in practice)
```

## Design decisions
- Pipeline: camera decode → grade → Master Amount → output. Master Amount scales only the grade; conversions stay active. Bypass is an exact copy.
- Unverifiable profiles are labelled experimental or pass the image through with a warning; no coefficient is guessed.
- Out-of-range values are preserved; NaN → 0, ±Inf → ±1e30; alpha untouched.
- Stateless renderer, `RenderFullySafe`; look LUTs are baked once and cached (thread-safe).

## Build

Requirements: CMake ≥ 3.21 and a C++17 compiler. No other dependencies (the SDK is vendored; no network needed).

| Platform | Command | Compiled by author? |
|---|---|---|
| Linux (development/unit tests) | `cmake --preset linux-dev && cmake --build --preset linux-dev && ctest --preset linux-dev` | **yes** (GCC 13.3) |
| Windows 10/11 x64, Visual Studio 2022 | `.\Tools\build_windows.ps1` | **no** |
| macOS Apple Silicon | `Tools/build_macos.sh arm64` | **no** |
| macOS Intel | `Tools/build_macos.sh x86_64` | **no** |
| macOS universal | `Tools/build_macos.sh universal` | **no** |

Bundle layout produced (`<build>/bundle/<config>/`):

```
RWD_COLOR_LAB.ofx.bundle/Contents/Win64/RWD_COLOR_LAB.ofx      (Windows)
RWD_COLOR_LAB.ofx.bundle/Contents/MacOS/RWD_COLOR_LAB.ofx      (macOS) + Contents/Info.plist
```

Install paths and troubleshooting: [Documentation/INSTALLATION_GUIDE.md](Documentation/INSTALLATION_GUIDE.md).

## Known limitations
- Not tested in DaVinci Resolve (19 or 21), Windows or macOS. No real camera footage was used.
- D-Log M, C-Log2, GP-Log unavailable; Canon gamut, BMD gamut, Rec.709-A experimental (see CAMERA_PROFILES).
- CPU only (≈1.9 s per 4K frame single-threaded in the sandbox); no GPU backend.
- Temperature/Tint are creative, not Kelvin-accurate. The 47 looks are original recipes, not film emulations.
