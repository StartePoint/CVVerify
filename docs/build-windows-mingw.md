# Windows MinGW Build

## Baseline

The maintained local toolchain is:

- Qt 5.14.2 desktop MinGW 64-bit
- MinGW 7.3.0 64-bit
- bundled OpenCV SDK at `opencvsdk/windows/opencv4.12`
- CMake 3.21+
- qmake from the same Qt installation

External OpenCV installation is not required when the bundled SDK is present.

## Repository Assumptions

The default OpenCV SDK root is:

- `opencvsdk/windows/opencv4.12`

Expected layout:

- `opencvsdk/windows/opencv4.12/include`
- `opencvsdk/windows/opencv4.12/x64/mingw/lib/OpenCVConfig.cmake`
- `opencvsdk/windows/opencv4.12/x64/mingw/bin`

If the bundled SDK version changes, update:

- `CVVERIFY_OPENCV_SDK_ROOT` in `CMakeLists.txt`
- `CVVERIFY_OPENCV_SDK_ROOT` in qmake invocations
- `THIRD_PARTY_NOTICES.md` if dependency notices changed

## Recommended Environment Variables

Run from the repository root:

```powershell
$env:CVVERIFY_QT_ROOT = "C:/Qt/Qt5.14.2/5.14.2/mingw73_64"
$env:CVVERIFY_MINGW_BIN = "C:/Qt/Qt5.14.2/Tools/mingw730_64/bin"
$env:CVVERIFY_OPENCV_SDK_ROOT = "$PWD/opencvsdk/windows/opencv4.12"
```

## CMake Configure

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH="$env:CVVERIFY_QT_ROOT" `
  -DCMAKE_C_COMPILER="$env:CVVERIFY_MINGW_BIN/gcc.exe" `
  -DCMAKE_CXX_COMPILER="$env:CVVERIFY_MINGW_BIN/g++.exe" `
  -DCMAKE_MAKE_PROGRAM="$env:CVVERIFY_MINGW_BIN/mingw32-make.exe" `
  -DCVVERIFY_QT_RUNTIME_BIN="$env:CVVERIFY_QT_ROOT/bin" `
  -DCVVERIFY_OPENCV_SDK_ROOT="$env:CVVERIFY_OPENCV_SDK_ROOT"
```

## CMake Build

```powershell
cmake --build build-mingw -j4
```

## CMake Test

```powershell
ctest --test-dir build-mingw --output-on-failure
```

## Run the Application

```powershell
.\build-mingw\CVVerify.exe
```

## qmake Configure

```powershell
New-Item -ItemType Directory -Force -Path build-qmake | Out-Null
Push-Location build-qmake
& "$env:CVVERIFY_QT_ROOT/bin/qmake.exe" ..\CVVerify.pro `
  "CVVERIFY_OPENCV_SDK_ROOT=$($env:CVVERIFY_OPENCV_SDK_ROOT -replace '\\','/')"
Pop-Location
```

## qmake Build

```powershell
Push-Location build-qmake
& "$env:CVVERIFY_MINGW_BIN/mingw32-make.exe" -j4
Pop-Location
```

## qmake Test Build and Run

```powershell
New-Item -ItemType Directory -Force -Path build-qmake-tests | Out-Null
Push-Location build-qmake-tests
& "$env:CVVERIFY_QT_ROOT/bin/qmake.exe" ..\CVVerifyCoreTests.pro `
  "CVVERIFY_OPENCV_SDK_ROOT=$($env:CVVERIFY_OPENCV_SDK_ROOT -replace '\\','/')"
& "$env:CVVERIFY_MINGW_BIN/mingw32-make.exe" -j4
.\CVVerifyCoreTests.exe
Pop-Location
```

## Runtime PATH Note

When launching the application or tests from a fresh shell, make sure `PATH` includes:

- `$env:CVVERIFY_OPENCV_SDK_ROOT/x64/mingw/bin`
- `$env:CVVERIFY_QT_ROOT/bin`
- `$env:CVVERIFY_MINGW_BIN`
