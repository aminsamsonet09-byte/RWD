# Build on Windows 10/11 (x64)

Easiest: let GitHub Actions do it — push the project to a GitHub repository, open **Actions → build**, download the artifact `RWD_COLOR_LAB_Windows_x64`.

Local build: Visual Studio 2022 ("Desktop development with C++") and CMake ≥ 3.21.
```
cmake --preset windows-msvc-x64
cmake --build --preset windows-msvc-x64 --config Release
ctest --preset windows-msvc-x64 -C Release
```
Result: `Builds\windows-x64\bundle\Release\RWD_COLOR_LAB.ofx.bundle` (binary in `Contents\Win64\`). `Tools\build_windows.ps1 [-Install]` wraps these steps. The MSVC runtime is linked statically.

Status: these commands were written but **never executed on Windows by the author**; the CI workflow is the intended first real build.
