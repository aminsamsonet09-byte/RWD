# Test Report (v0.3.0)

Environment: Linux x86-64 sandbox, GCC 13.3.0, Clang 18.1.3, CMake 3.28.3, Ninja. No Windows, no macOS, no DaVinci Resolve.

There are three separate categories. Only the first two were run.

## A. Unit tests (shared CPU engine) — RUN

`Tests/unit/` — 44 tests, 181,479 checks, **0 failures** (29 engine/renderer tests from v0.2 + 15 colour tests).

Configurations run:

| Config | Result |
|---|---|
| GCC 13.3, Release, `-Werror` | pass |
| Clang 18.1, Release, `-Werror` | pass |
| GCC 13.3, Debug, ASan + UBSan, `-Werror` | pass, no sanitizer reports |
| Clang 18.1, Release, `-Werror` | pass |
| Clang 18.1 + ASan/UBSan | **not run** (sanitizer runtime missing in sandbox) |

Coverage (requested items → test names):

| Requested | Tests |
|---|---|
| Exposure 0 / +1 / −1 / +2 stops | `exposure_zero_stops_is_exact_identity`, `exposure_plus_one_stop_doubles_rgb`, `exposure_minus_one_stop_halves_rgb`, `exposure_plus_two_stops_quadruples_rgb`; plus fractional stop and range clamping/NaN |
| Bypass | `bypass_output_is_bit_identical_to_input` (incl. NaN, ±Inf, −0), `bypass_works_for_integer_formats` |
| Alpha preservation | `alpha_is_preserved_bit_exact_float` (incl. NaN alpha), `alpha_is_preserved_integer_formats` |
| Negative RGB | `negative_rgb_is_scaled_not_clipped` |
| NaN / Inf | `nan_and_infinity_never_propagate` (3 EV × 3 Master Amount combinations) |
| Different dimensions | `different_image_dimensions` (1×1 … 257×129) |
| Row stride | `row_padding_is_respected_and_not_overwritten`, `negative_row_bytes_bottom_up_storage` |
| Master Amount | `master_amount_prepare_maps_and_clamps`, `master_amount_blends_between_input_and_graded` |
| Render window / bounds | window-only modification, clipping, non-zero origin, source smaller/partially outside destination, missing source |
| 8/16-bit | `uint8_exposure_rounds_and_clamps`, `uint16_exposure_rounds_and_clamps` |
| Robustness | invalid arguments, abort polling, in-place (aliased) rendering, disjoint windows = full render |

Colour tests (`Tests/unit/test_color.cpp`): every camera decoder round-trips against an independently written forward encoder; documented check values (S-Log3 95/420/598, S-Log2 90/347/582, F-Log 95/470/705, D-Log 95/408/586, V-Log 0.42331, C-Log3 0.34339, N-Log 0.36367, BMD Gen5 0.38356, C-Log 32.8 %) decode to 0 / 0.18 / 0.90; decoders monotonic and finite; D-Gamut and V-Gamut matrices equal the published ones (2e-3); white preserved and round-trip identity for all gamuts; output transfers (sRGB 0.18 → 0.46136, BT.2020, sign symmetry); 47 looks in the 10/7/6/6/8/10 collection split, unique names, finite, pairwise distinct; baked LUT vs direct evaluation (<1.2 %), negative/overflow handling; default settings are an exact identity; **camera+output conversion stay active at Master Amount 0 % and Intensity 0 %**; bypass and unavailable profiles; contrast pivot, saturation 0, temperature luma preserved, shadows/highlights, monotonicity; hostile NaN/Inf/negative input with every control on; Intensity and Master Amount blending.

Independent numerical cross-check of the published constants: `Tools/spec_check/verify_specs.py`.

Limits of these tests: they test the engine on test images I generated. They do not test the OFX glue (`rwd_effect.cpp` and `param_*`: image fetch/release, format validation, `isIdentity`, parameter layout, Prev/Next/Reset handlers), which needs a host.

## B. Plug-in load test — RUN (Linux only)

`Tests/plugin_load/` — loads the built `.ofx` with `dlopen`: exports `OfxGetNumberOfPlugins` and `OfxGetPlugin` (and nothing else), reports 1 plug-in, API `OfxImageEffectPluginAPI` v1, identifier `id.ridwanard.RWDColorLab`, version 0.3, has `setHost` and `mainEntry`. 11 checks, **0 failures**.

This proves the binary is a loadable OFX module. It does **not** call describe/render and says nothing about Resolve.

## C. DaVinci Resolve integration tests — NOT RUN

No Resolve was available. Everything about how Resolve loads the plug-in, draws its parameters and renders pixels is **unverified**, on both platforms and both Resolve versions. See COMPATIBILITY.md for the first checks to perform.

## Not tested at all

Windows build, macOS build (CI workflow written, never run), real camera footage (all curves tested only on synthetic values), visual quality of the looks (checked numerically, never judged on real images), memory behaviour under a real host, OFX describe/render calls (no host), and GPU (not implemented). Performance: ≈1.9 s per 4K float frame single-threaded in the sandbox.

## Reproduce

```
cmake --preset linux-dev && cmake --build --preset linux-dev && ctest --preset linux-dev
cmake --preset linux-asan && cmake --build --preset linux-asan && ctest --preset linux-asan
```
