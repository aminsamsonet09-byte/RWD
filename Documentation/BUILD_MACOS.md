# Build on macOS (Apple Silicon and Intel)

Easiest: GitHub Actions artifact `RWD_COLOR_LAB_macOS_universal` (runs on a macOS runner, arm64 + x86_64).

Local build: Xcode command-line tools, CMake ≥ 3.21.
```
cmake --preset macos-universal        # or macos-arm64 / macos-x86_64
cmake --build --preset macos-universal --config Release
ctest --preset macos-universal -C Release
```
Result: `Builds/macos-universal/bundle/Release/RWD_COLOR_LAB.ofx.bundle` (ad-hoc signed). `Tools/build_macos.sh universal [install]` wraps this.
For other Macs: sign with a Developer ID (`-DRWD_MACOS_CODESIGN_IDENTITY=...`) and notarize (not automated), or remove quarantine: `xattr -dr com.apple.quarantine /Library/OFX/Plugins/RWD_COLOR_LAB.ofx.bundle`.

Status: **never executed on macOS by the author.**
