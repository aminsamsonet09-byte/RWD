# Changelog

## 0.3.0 — Full feature set (2026-11-15)

Added
- Camera conversion: Rec.709 G2.4, Linear Rec.2020, Sony S-Log2 / S-Log3 (S-Gamut, S-Gamut3, S-Gamut3.Cine), DJI D-Log, Canon C-Log / C-Log3 (experimental), Panasonic V-Log, Fujifilm F-Log / F-Log2, Nikon N-Log, Blackmagic Film Gen 5 (experimental gamut). D-Log M, C-Log2 and GP-Log: unavailable (no confirmed spec) → pass-through with a warning. See CAMERA_PROFILES.md.
- Outputs: Rec.709 G2.4, G2.2, Rec.709-A (experimental), sRGB, Rec.2020 (BT.2020 OETF), Linear Rec.2020.
- Basic adjustments: Contrast, Saturation, Temperature, Tint, Highlights, Shadows (plus Exposure).
- 47 original Creative Looks in 6 collections, baked 65³ LUTs with log shaper + tetrahedral interpolation; Creative LUT dropdown, Prev LUT / Next LUT, Intensity.
- Reset button, Profile status label + persistent warning for experimental/unavailable profiles.
- GitHub Actions workflow building Windows x64 and macOS universal ZIPs.
- Docs: BUILD_WINDOWS, BUILD_MACOS, COLOR_SCIENCE, CAMERA_PROFILES, CREATIVE_LOOKS; `Tools/spec_check/verify_specs.py`.
- 15 colour unit tests (44 total).

Changed
- Pipeline is now camera decode → grade → Master Amount → output; Exposure acts on scene-linear data. With the default menus and neutral controls the plug-in is an exact identity.
- Master Amount scales the whole grade (adjustments + look); conversions are unaffected.
- Plug-in version 0.3.

Known issues
- Never built on Windows/macOS, never run in DaVinci Resolve. No GPU path. CPU speed ≈1.9 s per 4K frame single thread.

## 0.2.0 — Phase 2: OFX foundation and CPU engine (2026-11-13/14)

Added
- Vendored official OpenFX 1.5 SDK (headers + C++ Support library), unmodified.
- OFX plug-in `id.ridwanard.RWDColorLab` v0.2: describe, describeInContext, instance create/destroy, render, `isIdentity`.
- Parameters: Exposure (−4…+4 EV, default 0), Master Amount (0–100 %, default 100), Bypass. Grouped under "Global Controls".
- Platform-independent CPU engine: float32/uint16/uint8 RGBA, alpha untouched, render-window/tile aware, stride-safe (padding and negative row bytes), NaN/Inf/negative safe, abort polling per row.
- CMake build (Windows MSVC x64 static runtime, macOS arm64 / x86_64 / universal, Linux dev), presets, OFX bundle packaging, macOS Info.plist and ad-hoc codesign step.
- Tools: `build_windows.ps1`, `build_macos.sh` (not yet executed on those platforms).
- Tests: 29 engine unit tests, 1 plug-in load test.

Fixed during development
- `ParamSet` helper name collided with `OFX::ParamSet`; renamed `GradeParams`.
- Vendored Support and engine symbols were exported from the `.ofx`; now hidden, only `OfxGetNumberOfPlugins` and `OfxGetPlugin` are exported.
- Unused trait constants and two possibly-uninitialised variables found by `-Werror` builds were removed/initialised (not silenced).

Not done / known issues
- No Windows or macOS build has been run. No Resolve testing.
- Camera conversion, output conversion, 47 looks, contrast/saturation/temperature/tint/highlights/shadows, GPU: not started.

## 0.1.0 — Phase 1: architecture (2026-11-13)

- `Documentation/ARCHITECTURE.md`, folder structure. No code.
