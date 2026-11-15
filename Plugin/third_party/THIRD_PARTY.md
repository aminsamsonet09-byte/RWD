# Third-party code

## OpenFX 1.5 (vendored, unmodified)

- Project: https://github.com/AcademySoftwareFoundation/openfx
- Tag `OFX_Release_1.5`, commit `a98b77b0fd61b8dafb82f20e93d9b82ac97a55a5`
- License: BSD 3-Clause (`openfx/LICENSE.md`)
- Copied directories/files (no edits):
  - `openfx/include/*.h`                      OpenFX C API headers
  - `openfx/Support/include/*.h`             OpenFX C++ Support library headers
  - `openfx/Support/Library/*.cpp, *.h`      OpenFX C++ Support library sources
- Built as the static library `ofx_support` and linked into `RWD_COLOR_LAB.ofx`.
- Known SDK behaviour: `OfxGetPlugin(n)` does not bounds-check `n`; hosts must only
  request 0..OfxGetNumberOfPlugins()-1.

To update: replace the three copied locations from a newer official tag and update
the commit above. Do not hand-edit vendored files.
