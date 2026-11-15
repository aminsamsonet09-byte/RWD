# Compatibility Report (v0.3.0)

Honest status. A row is only "yes" if it was actually done.

| Target | Compiled | Loaded by Resolve | Rendered in Resolve | Notes |
|---|---|---|---|---|
| Linux x86-64, GCC 13.3 (dev only) | **yes** (`-Wall -Wextra -Wpedantic -Werror`) | n/a | n/a | Resolve not run. Loaded via `dlopen` by the load test only. |
| Linux, Clang 18.1 | **yes** (engine + unit tests) | n/a | n/a | Plug-in not built with Clang. |
| Windows 10/11 x64 (MSVC, VS 2022) | not tested (CI workflow written, not yet run) | not tested | not tested | Build script, CMake and `.github/workflows/build.yml` written, never run. |
| macOS Apple Silicon (arm64) | not tested (CI workflow written, not yet run) | not tested | not tested | Never run. |
| macOS Intel (x86_64) | not tested | not tested | not tested | Same. Universal build needs both slices to build. |
| DaVinci Resolve Studio 19 | – | not tested | not tested | |
| DaVinci Resolve Studio 21 | – | not tested | not tested | |

## OpenFX facts used

- SDK: official OpenFX 1.5 (tag `OFX_Release_1.5`). Uses only long-standing image-effect APIs (clips, double/boolean params, groups, pages, `isIdentity`, render with tiles).
- Declares contexts Filter and General; RGBA only; 8-bit, 16-bit, float; `RenderFullySafe`.
- Resolve's OFX host is not the reference host: differences in Inspector rendering (group/page layout), identity handling and pixel depth delivered are **unverified**.

## Things to check first when testing in Resolve

1. Does the plug-in appear under OpenFX → RWD?
2. Do the four groups (Conversion, Creative LUT, Basic Adjustments, Global Controls) appear, do the dropdowns list all entries, do Prev/Next/Reset buttons work, and does the Profile status text update?
3. Is Bypass visually identical to the node disabled?
4. Does 8-bit media render correctly (no banding added at 0 EV)?
5. Same on Resolve 19 and 21, Windows and macOS.

## Not applicable yet

GPU backends (Metal / CUDA / OpenCL): **not implemented**. The CPU path is the only path.
