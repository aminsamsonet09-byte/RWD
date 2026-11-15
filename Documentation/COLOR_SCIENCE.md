# Color Science

## Pipeline (per pixel, CPU)
1. Sanitise (NaN → 0, ±Inf → ±1e30). Alpha is never touched.
2. **Camera decode**: transfer function → scene-linear, then 3×3 gamut matrix → working space.
3. **Grade** (only when something is non-neutral): Exposure → Temperature/Tint → Contrast → Shadows/Highlights → Saturation → Creative LUT × Intensity.
4. **Master Amount**: `result = mix(ungraded, graded, amount)` in the working space. Step 2 and step 5 never depend on it.
5. **Output**: matrix to the output gamut → transfer encode.

Bypass copies the input bit-exactly. If both menus are at their defaults (Rec.709 G2.4 → Rec.709 G2.4) and nothing is adjusted, the plug-in is an exact identity and tells the host so.

## Working space
Linear Rec.2020 primaries, D65 white. All gamut matrices are **derived from published chromaticities** (RGB→XYZ from primaries and white, Bradford adaptation only if a gamut's white is not D65). D-Gamut and V-Gamut results are unit-tested against the matrices printed in the DJI and Panasonic documents; white (1,1,1) maps to (1,1,1) for every gamut.

## Operations
- **Exposure**: gain 2^EV on linear data.
- **Contrast**: luma-ratio power about 0.18 (`k = 0.18·(Y/0.18)^p / Y`, p = 2^(c/100)); hue preserving; pivot exact.
- **Temperature / Tint**: per-channel gains, normalised to keep Rec.2020 luma — a creative shift, **not** Kelvin-accurate white balance.
- **Highlights / Shadows**: logistic masks over stops about mid grey, up to ±1 stop; monotonic (tested).
- **Saturation**: lerp between Rec.2020 luma and colour; 0 % gives neutral grey.
- **Creative LUT**: see CREATIVE_LOOKS.md.

## Known limits
- No tone mapping: scene 0.18 maps to display 0.18 linear, so log footage looks flat-to-contrasty depending on camera; use the look/contrast controls or a tone-mapping node.
- Math is float32; log decode/encode uses double precision internally.
- Performance: the CPU path measured ≈1.9 s for one 4K float frame on a single thread (look + conversion active) in the Linux dev sandbox. Resolve renders tiles on several threads, but real-time playback at 4K is **not** expected. GPU backends (Metal/CUDA/OpenCL) are **not implemented**; the CPU renderer is the only path.
