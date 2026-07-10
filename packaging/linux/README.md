# Linux packaging

## Prerequisites

- Release build at `build-linux/CVVerify` (or your chosen build directory)
- Qt 5.14+ shared libraries on the target machine or copied via `CVVERIFY_QT_ROOT`
- OpenCV shared libraries available through `CVVERIFY_OPENCV_LIB_DIR` when not linked statically

## Package

From the repository root:

```bash
export CVVERIFY_QT_ROOT=/opt/Qt/5.14.2/gcc_64
export CVVERIFY_OPENCV_LIB_DIR=/opt/opencv/lib
chmod +x packaging/linux/package.sh
./packaging/linux/package.sh build-linux dist/CVVerify
```

## Output layout

```text
dist/CVVerify/
  CVVerify
  run-cvverify.sh
  config/
  samples/
  libQt5*.so*
  libopencv_*.so*
  platforms/
```

Launch with `./dist/CVVerify/run-cvverify.sh` so bundled libraries resolve through `LD_LIBRARY_PATH`.

## Notes

- Adjust `build-linux` to match your CMake build folder name.
- For AppImage or distro packages, wrap this layout with your preferred bundler.
- Runtime resources resolve from `config/` next to the executable via `PlatformPaths::resolveResourcePath`.
