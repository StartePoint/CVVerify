# Linux Build (Cross-Platform Oriented)

CVVerify is developed and validated primarily on Windows + Qt 5.14.2 MinGW. Linux builds follow the same source layout but require a locally installed Qt 5.14+ and OpenCV 4.x development packages.

## Prerequisites

- GCC or Clang with C++17 support
- CMake 3.21+
- Qt 5.14+ (`Qt5Core`, `Qt5Gui`, `Qt5Widgets`)
- OpenCV 4.x with `core`, `imgproc`, `imgcodecs`, `videoio`, and `dnn` modules

## Configure

```bash
export CVVERIFY_QT_ROOT=/opt/Qt/5.14.2/gcc_64
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$CVVERIFY_QT_ROOT" \
  -DCVVERIFY_OPENCV_SDK_ROOT=/usr/local
```

Set `CVVERIFY_OPENCV_SDK_ROOT` to your OpenCV install prefix or a vendored SDK tree that provides `OpenCVConfig.cmake`.

## Build and Test

```bash
cmake --build build-linux -j$(nproc)
ctest --test-dir build-linux --output-on-failure
```

## Run

```bash
./build-linux/CVVerify
```

CMake copies `config/` and `samples/` beside the executable after each build.

## Package

See [packaging/linux/README.md](../packaging/linux/README.md) for the `package.sh` helper.
