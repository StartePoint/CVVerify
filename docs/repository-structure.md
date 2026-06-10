# Repository Structure

This document explains how the repository is organized so that new contributors can quickly place code in the right layer.

## Top-Level Directories

- `src/`: application source code.
- `tests/`: unit tests for core logic and UI components.
- `resources/`: icons, resource collections, and Windows resource files.
- `docs/`: build instructions, review artifacts, design documents, and repository guidance.
- `qmake/`: shared qmake include files and explicit source lists.
- `packaging/`: release packaging helpers and related assets.
- `samples/`: sample content reserved for future example inputs.
- `opencvsdk/`: bundled Windows MinGW OpenCV SDK tracked by the repository.

## Source Layout

- `src/app/`: startup configuration and persisted application settings.
- `src/core/`: media sources, operators, detection flow, and pipeline domain logic.
- `src/infra/`: framework or vendor integration helpers such as OpenCV/Qt conversion utilities.
- `src/ui/`: main window, dialogs, panels, and widgets.

Keep algorithmic processing in `src/core` whenever possible. Use `src/infra` for adapter code and `src/ui` for QWidget behavior and presentation logic.

## Test Layout

- `tests/unit/core/`: unit tests for pipeline, media, registry, serialization, and detection logic.
- `tests/unit/ui/`: unit tests for panels, dialogs, the main window, and canvas behavior.

The aggregated executable name is `CVVerifyCoreTests` for both CMake and qmake workflows.

## Build Files

- `CMakeLists.txt`: primary CMake build entry point.
- `CVVerify.pro`: qmake application build entry point.
- `CVVerifyCoreTests.pro`: qmake test build entry point.
- `qmake/*.pri`: explicit source lists used by qmake builds.

## Documents Worth Reading First

- `README.md`
- `docs/build-windows-mingw.md`
- `CONTRIBUTING.md`
- `THIRD_PARTY_NOTICES.md`
- `docs/open-source-release-checklist.md`
