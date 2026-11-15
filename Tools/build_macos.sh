#!/usr/bin/env bash
# RWD COLOR LAB - macOS build (Xcode command line tools + CMake).
#
#   Tools/build_macos.sh                 # universal (arm64 + x86_64), build + test
#   Tools/build_macos.sh arm64           # Apple Silicon only
#   Tools/build_macos.sh x86_64          # Intel only
#   Tools/build_macos.sh universal install   # also install to /Library/OFX/Plugins (sudo)
#
# STATUS: written but NOT yet executed on a Mac by the author. Report any error you hit.
set -euo pipefail

ARCH="${1:-universal}"
INSTALL="${2:-}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

command -v cmake >/dev/null || { echo "cmake not found (brew install cmake)"; exit 1; }
command -v clang++ >/dev/null || { echo "Xcode command line tools missing (xcode-select --install)"; exit 1; }

case "$ARCH" in
  universal) ARCHS="arm64;x86_64" ;;
  arm64|x86_64) ARCHS="$ARCH" ;;
  *) echo "unknown architecture '$ARCH' (universal|arm64|x86_64)"; exit 1 ;;
esac

BUILD="$ROOT/Builds/macos-$ARCH"
echo "==> Configure ($ARCHS)"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$ARCHS"
echo "==> Build"
cmake --build "$BUILD" --parallel
echo "==> Test"
# The load test can only dlopen a slice that matches the machine it runs on.
ctest --test-dir "$BUILD" --output-on-failure

BUNDLE="$BUILD/bundle/Release/RWD_COLOR_LAB.ofx.bundle"
[ -f "$BUNDLE/Contents/MacOS/RWD_COLOR_LAB.ofx" ] || { echo "Bundle was not produced"; exit 1; }
echo "==> Bundle: $BUNDLE"
lipo -info "$BUNDLE/Contents/MacOS/RWD_COLOR_LAB.ofx"
codesign --verify --verbose=2 "$BUNDLE" || echo "(codesign verification reported a problem)"

if [ "$INSTALL" = "install" ]; then
  echo "==> Install to /Library/OFX/Plugins (sudo)"
  sudo mkdir -p /Library/OFX/Plugins
  sudo rm -rf "/Library/OFX/Plugins/RWD_COLOR_LAB.ofx.bundle"
  sudo cp -R "$BUNDLE" /Library/OFX/Plugins/
  echo "Done. Restart DaVinci Resolve."
fi
