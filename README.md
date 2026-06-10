# CVVerify

CVVerify is a Qt Widgets desktop workstation for validating computer vision pipelines on Windows-first toolchains. It combines traditional OpenCV operators, image and video preview, schema-driven parameter editing, pipeline composition, and ONNX-oriented model workflows in one application shell.

## What CVVerify Covers Today

- Image and video loading with a shared frame packet abstraction.
- Traditional operator categories for morphology, filtering, enhancement and transforms, thresholding, edge detection, visual effects, detection, segmentation, and feature workflows.
- Embedded parameter editing for pipeline steps without opening a parameter dialog for every operator.
- Full-pipeline and single-node preview modes.
- Export-oriented detection workflow scaffolding, including YOLO model packaging and result composition.
- Unit tests covering core pipeline logic and major UI panels.

Some feature areas are still evolving. The repository already contains placeholder implementations for selected SURF-related workflows where upstream availability, licensing, or deployment constraints need to be handled carefully.

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

Related documentation:

- [Repository Structure](docs/repository-structure.md)
- [Open Source Release Checklist](docs/open-source-release-checklist.md)
- [Third-Party Notices](THIRD_PARTY_NOTICES.md)
- [Operator Parameter Review](docs/operator-parameter-review.html)

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

## Repository Layout

- `src/app`: application settings and startup glue.
- `src/core`: media, operators, detection, and pipeline logic.
- `src/infra`: OpenCV and framework integration helpers.
- `src/ui`: main window, dialogs, panels, and widgets.
- `tests/unit`: core and UI unit tests collected into `CVVerifyCoreTests`.
- `resources`: icons, Windows resource files, and application resources.
- `qmake`: shared qmake include files and source lists.
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

This repository is still pre-release, but it already contains a usable vertical slice:

- main window shell with dual preview areas
- pipeline strip and embedded operator workbench
- localized menus and parameter labels
- image/video media loading
- schema-driven operator registry
- YOLO package import scaffolding
- unit test harness for core and UI modules

GitHub Actions currently validate repository metadata and contribution files. Full hosted build CI is intentionally deferred until the Windows Qt 5.14.2 + MinGW provisioning path is stabilized for public runners.

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
