# Camera & Output Profiles

Status meanings: **Verified** = constants taken from the manufacturer's own document and checked numerically · **Reference** = matches published reference-implementation values / check values, but not read in the maker's own document during this work · **Experimental** = part of the definition is unconfirmed, colours may be inexact (marked "(experimental)" in the menu and a warning is shown) · **Unavailable** = no public specification found; the plug-in passes the image through unchanged and shows a warning.

All decoders are normalised so that **0.18 = mid grey** (scene reflection, 0.9 = white) and follow the documented 10-bit code mapping (code/1023).

## Input ("Conversion LUT") — menu index is saved in projects, append-only

| # | Menu entry | Status | Notes |
|---|---|---|---|
| 0 | Rec.709 Gamma 2.4 (no camera conversion) | Verified | Default. Display-referred footage. |
| 1 | Linear Rec.2020 (EXR / linear) | Verified | No conversion. |
| 2 | Sony S-Log2 / S-Gamut | Reference | Assumes legal-range code mapping (S-Log2 0 % = code 90). Full-range 8-bit clips would be off; not confirmed on real A6400 files. |
| 3 | Sony S-Log2 / S-Gamut3.Cine | Reference | Same curve, S-Gamut3.Cine primaries. |
| 4 | Sony S-Log3 / S-Gamut3 | Verified | Sony technical summary. Check codes 95/420/598 → 0/0.18/0.90. |
| 5 | Sony S-Log3 / S-Gamut3.Cine | Verified | |
| 6 | DJI D-Log / D-Gamut | Verified | DJI whitepaper; D-Gamut matrix matches the printed matrix to 2e-4. |
| 7 | DJI D-Log M | **Unavailable** | D-Log M is not identical to D-Log and no public specification was found. Pass-through. If you can provide DJI's D-Log M document or an official CST LUT, it can be added. |
| 8 | Canon C-Log / Cinema Gamut | Experimental | C-Log curve reproduces Canon's 18 % = 32.8 %; Cinema Gamut white point unconfirmed (D55 assumed, Bradford-adapted). |
| 9 | Canon C-Log3 / Cinema Gamut | Experimental | C-Log3 curve matches reference (0.18 → 0.34339); gamut caveat as above. |
| 10 | Canon C-Log2 | **Unavailable** | Constants could not be confirmed. |
| 11 | Panasonic V-Log / V-Gamut | Reference | V-Gamut matrix matches Panasonic's printed matrix to 2e-4; V-Log 0.18 → 0.42331. |
| 12 | Fujifilm F-Log / F-Gamut | Verified | Fujifilm data sheet; F-Gamut = BT.2020 primaries. |
| 13 | Fujifilm F-Log2 / F-Gamut | Reference | Matches published check values (codes 95/400/570). |
| 14 | Nikon N-Log / BT.2020 | Verified | Nikon N-Log specification (BT.2020, D65). |
| 15 | Blackmagic Film Gen 5 / Wide Gamut | Experimental | Curve checked (0.18 → 0.38356); gamut primaries not cross-checked. |

Not offered: **GoPro GP-Log** (no public specification found for HERO11–13).

Sony A6400 8-bit: use entries 2–5. 8-bit log has little precision: expect banding when pushing hard. The correct entry depends on the Picture Profile (PP7 = S-Log2, PP8/PP9 = S-Log3, gamut per PP); check your camera menu — not verified against A6400 footage here.

## Output ("Conversion output")

| # | Entry | Status |
|---|---|---|
| 0 | Rec.709 Gamma 2.4 | Verified (pure power 1/2.4 encode) |
| 1 | Rec.709 Gamma 2.2 | Verified |
| 2 | Rec.709-A (Apple, gamma ≈ 1.96) | **Experimental** — exact Apple definition not confirmed; implemented as pure power 1/1.961 |
| 3 | sRGB | Verified (IEC 61966-2-1 piecewise) |
| 4 | Rec.2020 (BT.2020 OETF) | Verified (explicit transfer, α=1.09929682680944) |
| 5 | Linear Rec.2020 | Verified |

Output transforms are colorimetric: no tone mapping or gamut compression. Out-of-gamut values after the matrix are kept (sign-symmetric encode) and clipped only by 8/16-bit storage.
