<#
  RWD COLOR LAB - Windows build (Visual Studio 2022, x64).
  Run in "Developer PowerShell for VS 2022" or any PowerShell with CMake on PATH.

  .\Tools\build_windows.ps1              # configure + build + test (Release)
  .\Tools\build_windows.ps1 -Install     # also copy the bundle to
                                         #   C:\Program Files\Common Files\OFX\Plugins
                                         #   (run PowerShell as Administrator)

  STATUS: written but NOT yet executed on Windows (no Windows machine was
  available to the author). Report any error you hit.
#>
param(
  [ValidateSet('Release', 'Debug')] [string]$Config = 'Release',
  [switch]$Install,
  [switch]$SkipTests
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'Builds\windows-x64'

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  throw 'cmake not found. Install CMake 3.21+ (or Visual Studio 2022 with the "C++ CMake tools" component).'
}

Write-Host '==> Configure (Visual Studio 17 2022, x64)'
cmake -S $root -B $build -G 'Visual Studio 17 2022' -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }

Write-Host "==> Build ($Config)"
cmake --build $build --config $Config --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }

if (-not $SkipTests) {
  Write-Host '==> Test'
  ctest --test-dir $build -C $Config --output-on-failure
  if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
}

$bundle = Join-Path $build "bundle\$Config\RWD_COLOR_LAB.ofx.bundle"
Write-Host "==> Bundle: $bundle"
if (-not (Test-Path (Join-Path $bundle 'Contents\Win64\RWD_COLOR_LAB.ofx'))) { throw 'Bundle was not produced' }

if ($Install) {
  $dest = 'C:\Program Files\Common Files\OFX\Plugins'
  Write-Host "==> Install to $dest (needs Administrator)"
  Copy-Item -Recurse -Force $bundle $dest
  Write-Host 'Done. Restart DaVinci Resolve. If the plug-in does not appear, delete OFXPluginCacheV2.xml (see INSTALLATION_GUIDE.md).'
}
