# CVVerify

CVVerify is a Qt Widgets desktop workstation for validating computer vision pipelines on Windows-first toolchains. It combines traditional OpenCV operators, image and video preview, schema-driven parameter editing, pipeline composition, and ONNX-oriented model workflows in one application shell.

## What CVVerify Covers Today

- Image, video, and image-folder loading with a shared `FramePacket` abstraction.
- Traditional operator categories for morphology, filtering, enhancement and transforms, thresholding, edge detection, visual effects, detection, segmentation, and feature workflows.
- Embedded parameter editing for pipeline steps without opening a parameter dialog for every operator.
- Full-pipeline and single-node preview modes with dual, wipe, and four-grid comparison views.
- Multi-type ONNX import (detection, classification, segmentation, OCR, keypoint, custom tensor).
- Model configuration editor with live preview and template save/load.
- Task history, diagnostics, statistics, and batch/offline export workflows.
- Comparison baseline capture for A/B operator or configuration validation.
- Histogram operators with optional sidecar chart mode.
- Operator JSON schemas under `config/operator_schemas/` for documentation and validation.
- Unit tests covering core pipeline logic and major UI panels.

Some feature areas are still evolving. SURF-related operators remain placeholders where upstream availability, licensing, or deployment constraints need careful handling.

## Toolchain Baseline

The current development baseline is pinned to:

- Qt 5.14.2
- MinGW 7.3.0 64-bit
- bundled OpenCV 4.12.0 SDK
- CMake and qmake project files

The repository is structured for future cross-platform work, but the actively maintained local workflow is Windows + Qt 5.14.2 + MinGW.

## Quick Start

Start with the Windows build guide:

- [Windows MinGW Build](docs/build-windows-mingw.md)

Cross-platform build notes:

- [Linux Build](docs/build-linux.md)
- [macOS Build](docs/build-macos.md)

Related documentation:

- [Quick Start Validation Workflows](docs/quick-start-validation.md)
- [Manual Validation Checklist (v0.1.0)](docs/manual-validation-v0.1.0.md)
- [Repository Structure](docs/repository-structure.md)
- [Open Source Release Checklist](docs/open-source-release-checklist.md)
- [Third-Party Notices](THIRD_PARTY_NOTICES.md)
- [Operator Parameter Review](docs/operator-parameter-review.html)
- [Operator Schemas](config/operator_schemas/README.md)
- [Sample Assets](samples/README.md)

## Build and Test Entry Points

CMake:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw --output-on-failure
```

qmake:

```powershell
qmake CVVerify.pro
mingw32-make
qmake CVVerifyCoreTests.pro
mingw32-make
.\CVVerifyCoreTests.exe
```

Use the detailed commands in [docs/build-windows-mingw.md](docs/build-windows-mingw.md) to provide the correct Qt, MinGW, and OpenCV paths.

Build outputs copy `config/` and `samples/` beside the executable automatically (CMake post-build step).

## Windows Packaging

See [packaging/windows/README.md](packaging/windows/README.md) for the `package.ps1` script that bundles the executable, Qt/OpenCV runtimes, and resource folders into `dist/CVVerify`.

Linux and macOS packaging helpers:

- [packaging/linux/README.md](packaging/linux/README.md)
- [packaging/macos/README.md](packaging/macos/README.md)

Maintenance scripts for bundled resources:

- [scripts/README.md](scripts/README.md)

## Repository Layout

- `src/app`: application settings, service locator, and command dispatch.
- `src/core`: media, operators, detection, pipeline, project, and task logic.
- `src/infra`: OpenCV and platform path integration helpers.
- `src/ui`: main window, dialogs, panels, and widgets.
- `config/`: app defaults, model templates, and operator schemas.
- `samples/`: example pipelines and project descriptors.
- `tests/unit`: core and UI unit tests collected into `CVVerifyCoreTests`.
- `resources`: icons, Windows resource files, and application resources.
- `qmake`: shared qmake include files and source lists.
- `packaging/`: Windows/Linux/macOS packaging notes and scripts.
- `docs`: build instructions, review artifacts, design documents, and repository guidance.

The longer explanation lives in [docs/repository-structure.md](docs/repository-structure.md).

## Bundled Dependencies

The repository tracks a bundled Windows MinGW OpenCV SDK at `opencvsdk/windows/opencv4.12`. That layout is the default dependency root used by both CMake and qmake.

When upgrading or replacing the bundled SDK:

- keep the folder under `opencvsdk/windows/`
- update `CVVERIFY_OPENCV_SDK_ROOT` in `CMakeLists.txt` if the versioned directory changes
- review [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)
- verify redistribution terms for the bundled binaries and notices

## Current Project Status

This repository is pre-release v0.1.0 with a broad V1 workstation slice implemented:

- main window with dual/wipe/four-grid preview and comparison baseline
- pipeline strip, embedded operator workbench, and parameter presets
- localized menus and parameter labels (English / Simplified Chinese)
- timeline scrubber labels and settings reset-to-defaults workflow
- multi-type ONNX model import, config editor, and model list panel
- detection/classification/segmentation/OCR/keypoint/custom tensor inference routing
- image/video/folder export with tensor summaries and optional side-by-side video output
- task history, diagnostics, and statistics panels
- operator schema files and packaging scripts for Windows, Linux, and macOS releases
- quick-start validation guide and bundled sample images/pipelines

GitHub Actions validate repository metadata and run a MinGW CMake build + `ctest` on `main` when Qt provisioning is available on the runner.

## Project Documents

- [Design Spec](docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md)
- [Foundation Plan](docs/superpowers/plans/2026-06-04-cvverify-v1-foundation-plan.md)
- [Media And Operator Plan](docs/superpowers/plans/2026-06-04-cvverify-media-operator-plan.md)
- [Operator System Expansion Plan](docs/superpowers/plans/2026-06-09-operator-system-expansion-plan.md)

## Open Source Collaboration

Repository collaboration files are already included:

- [Contributing Guide](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Security Policy](SECURITY.md)
- [Support Guide](SUPPORT.md)
- [Pull Request Template](.github/pull_request_template.md)

## License

CVVerify source code is licensed under the [MIT License](LICENSE). Bundled third-party components keep their own licenses and notices.
