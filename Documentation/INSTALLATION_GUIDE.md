# Installation Guide

There is no installer. Get the bundle from the GitHub Actions artifact (see BUILD_WINDOWS.md / BUILD_MACOS.md) or build it yourself, then copy it.
The bundle is the folder `RWD_COLOR_LAB.ofx.bundle`. Copy the **whole folder**.

## Windows 10/11 (x64)

1. Build: `.\Tools\build_windows.ps1` (Visual Studio 2022 with "Desktop development with C++", CMake 3.21+).
2. Copy `Builds\windows-x64\bundle\Release\RWD_COLOR_LAB.ofx.bundle` to
   `C:\Program Files\Common Files\OFX\Plugins\` (Administrator rights needed), or run the script with `-Install` in an Administrator PowerShell.
3. Restart DaVinci Resolve.

The MSVC runtime is linked statically by default, so the Visual C++ Redistributable is not required.

## macOS (Apple Silicon or Intel)

1. Build: `Tools/build_macos.sh universal` (or `arm64` / `x86_64`). Requires Xcode command line tools and CMake.
2. Copy `Builds/macos-<arch>/bundle/Release/RWD_COLOR_LAB.ofx.bundle` to `/Library/OFX/Plugins/` (or run `Tools/build_macos.sh universal install`).
3. Restart DaVinci Resolve.

The bundle is ad-hoc signed, which is enough to run on the Mac that built it. To give it to other Macs, build with `-DRWD_MACOS_CODESIGN_IDENTITY="Developer ID Application: …"` and notarize it (not automated). If macOS blocks a downloaded copy: `xattr -dr com.apple.quarantine /Library/OFX/Plugins/RWD_COLOR_LAB.ofx.bundle`.

## Using it in Resolve

Color page → OpenFX panel → **RWD** group → **RWD COLOR LAB** → drag onto a node. Resolve free may not load OpenFX plug-ins at all (OpenFX is a Studio feature in current versions — verify for your version). The plug-in is CPU-only.

Typical setup for Sony A6400 S-Log3: Conversion LUT = *Sony S-Log3 / S-Gamut3*, Conversion output = *Rec.709 Gamma 2.4*, pick a Creative LUT (Prev/Next to browse), set Intensity. Do not also apply a Resolve colour-space transform on the same node.

## If the plug-in does not appear

- Confirm the folder path and that the binary is inside `Contents/Win64/` (Windows) or `Contents/MacOS/` (macOS).
- Resolve caches the plug-in list; close Resolve and delete `OFXPluginCacheV2.xml` (location varies by OS/version; search your Resolve support folder), then restart.
- Check the Resolve log for an OFX load error and report it.

## Uninstall

Delete `RWD_COLOR_LAB.ofx.bundle` from the plug-in folder and restart Resolve.

*Paths are from the OpenFX packaging specification. They have not yet been exercised with Resolve by the author.*
