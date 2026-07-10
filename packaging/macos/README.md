# macOS packaging

## Prerequisites

- Release build producing `CVVerify` or `CVVerify.app`
- Qt 5.14+ with `macdeployqt` available under `CVVERIFY_QT_ROOT/bin`

## Package

From the repository root:

```bash
export CVVERIFY_QT_ROOT="$HOME/Qt/5.14.2/clang_64"
chmod +x packaging/macos/package.sh
./packaging/macos/package.sh build-macos dist/CVVerify.app
```

If CMake emits a flat binary at `build-macos/CVVerify`, the script creates a minimal `.app` bundle automatically.

## Output layout

```text
dist/CVVerify.app/
  Contents/
    MacOS/CVVerify
    Resources/
      config/
      samples/
    Frameworks/   (populated by macdeployqt)
```

## Notes

- `config/` and `samples/` are copied into `Contents/Resources/` for portable validation workflows.
- Sign and notarize the `.app` before public distribution.
- Runtime resources also resolve from paths beside the executable through `PlatformPaths::resolveResourcePath`.
