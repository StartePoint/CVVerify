# Windows packaging

## Prerequisites

- Release build at `build-mingw/CVVerify.exe`
- Qt 5.14.2 MinGW 64-bit (`CVVERIFY_QT_ROOT`)
- Bundled OpenCV runtime DLLs (copied automatically from `opencvsdk/windows/opencv4.12/x64/mingw/bin`)

## Package

From the repository root:

```powershell
$env:CVVERIFY_QT_ROOT = "C:/Qt/Qt5.14.2/5.14.2/mingw73_64"
.\packaging\windows\package.ps1 -BuildDir build-mingw -OutputDir dist/CVVerify
```

## Output layout

```text
dist/CVVerify/
  CVVerify.exe
  config/
  samples/
  Qt*.dll
  opencv_*.dll
  platforms/
```

Distribute the entire `dist/CVVerify` folder.

## Notes

- The packaging script runs `windeployqt` to collect Qt runtime dependencies.
- Runtime resources resolve from `config/` next to the executable via `PlatformPaths::resolveResourcePath`.
- For signed releases, add code signing after packaging.
