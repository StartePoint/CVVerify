param(
    [string]$BuildDir = "build-mingw",
    [string]$OutputDir = "dist/CVVerify",
    [string]$QtRoot = $env:CVVERIFY_QT_ROOT,
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$exePath = Join-Path $repoRoot "$BuildDir/CVVerify.exe"

if (-not (Test-Path $exePath)) {
    throw "Executable not found: $exePath. Build the project first."
}

if ([string]::IsNullOrWhiteSpace($QtRoot)) {
  $QtRoot = "C:/Qt/Qt5.14.2/5.14.2/mingw73_64"
}

$windeployqt = Join-Path $QtRoot "bin/windeployqt.exe"
if (-not (Test-Path $windeployqt)) {
    throw "windeployqt not found at $windeployqt. Set CVVERIFY_QT_ROOT."
}

$targetDir = Join-Path $repoRoot $OutputDir
if (Test-Path $targetDir) {
    Remove-Item -Recurse -Force $targetDir
}
New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

Copy-Item $exePath $targetDir
Copy-Item -Recurse (Join-Path $repoRoot "config") (Join-Path $targetDir "config")
Copy-Item -Recurse (Join-Path $repoRoot "samples") (Join-Path $targetDir "samples")

$opencvBin = Join-Path $repoRoot "opencvsdk/windows/opencv4.12/x64/mingw/bin"
if (Test-Path $opencvBin) {
    Copy-Item (Join-Path $opencvBin "*.dll") $targetDir
}

& $windeployqt --release --compiler-runtime --no-translations (Join-Path $targetDir "CVVerify.exe")

Write-Host "Packaged CVVerify to $targetDir"
