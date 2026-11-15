# Creative Looks (47)

Every look is an **original recipe written for this plug-in** (exposure, contrast about mid grey, highlight shoulder, hue rotation, shadow/highlight-weighted saturation, shadow/mid/highlight tints, faded blacks). They are *inspired by* the general mood their collection name suggests. **They are not measured emulations of any film stock, camera or product**, and no trademarked product name is used as a look name.

How they run: each recipe is baked once per session into a 65³ LUT over a log shaper (16 stops), applied with tetrahedral interpolation in the scene-linear Rec.2020 working space, after the basic adjustments and before Master Amount and the output transform. Intensity blends the look with the un-looked image. Negative values are carried through additively; values above 64 are extended, not clipped. Interpolation error versus direct evaluation is below ~1.2 % (unit test `lut_matches_direct_evaluation_and_extends_out_of_range`).

Distinctness is enforced by a unit test: the smallest RMS distance between any two looks on a patch set is above 0.015 in display gamma (currently 0.019).

Pick looks with the **Creative LUT** dropdown or **Prev LUT / Next LUT** (cycles None + 47). Choice index 0 = None.


## Cinematic

1. Teal & Amber
2. Blockbuster Punch
3. Noir Steel
4. Golden Hour Epic
5. Cold Thriller
6. Desert Dust
7. Neon Night
8. Muted Drama
9. Green Matrix
10. Soft Epic Fade

## Kodak-inspired

11. Amber Negative
12. Soft Portrait Film
13. Rich Chrome Slide
14. Daylight Print
15. Tungsten Cinema
16. Golden Reversal
17. Heritage Colour

## Fujifilm-inspired

18. Cool Green Negative
19. Velvet Slide
20. Clean Foliage
21. Pastel Negative
22. Muted Documentary
23. Teal Sky Chrome

## Vintage

24. Faded 70s
25. Sepia Archive
26. Super 8 Warm
27. Instant Wash
28. Cross-Process Retro
29. Old Newsroom

## Commercial / Fashion

30. Clean Commercial
31. High-Key Beauty
32. Bright Lifestyle
33. Magazine Punch
34. Skin Glow
35. Pastel Fashion
36. Crisp Product
37. Urban Streetwear

## Mood

38. Melancholy Blue
39. Warm Nostalgia
40. Dreamy Haze
41. Dark Moody
42. Romantic Rose
43. Autumn Calm
44. Winter Frost
45. Sunset Fire
46. Mono Silver
47. Mono Warm

*Judge them on your own footage: their look depends on how you set the camera input, exposure and output transform.*
