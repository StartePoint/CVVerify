# macOS Build (Cross-Platform Oriented)

CVVerify targets Qt Widgets and OpenCV DNN. macOS builds are supported at the architecture level; the maintained release workflow remains Windows-first.

## Prerequisites

- Xcode command-line tools
- CMake 3.21+
- Qt 5.14+ for macOS (`clang_64` kit)
- OpenCV 4.x built for the same architecture

## Configure

```bash
export CVVERIFY_QT_ROOT="$HOME/Qt/5.14.2/clang_64"
cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$CVVERIFY_QT_ROOT" \
  -DCVVERIFY_OPENCV_SDK_ROOT=/usr/local
```

## Build and Test

```bash
cmake --build build-macos -j$(sysctl -n hw.ncpu)
ctest --test-dir build-macos --output-on-failure
```

## Run

```bash
./build-macos/CVVerify
```

## Package

See [packaging/macos/README.md](../packaging/macos/README.md) for the `package.sh` helper and `macdeployqt` integration.
