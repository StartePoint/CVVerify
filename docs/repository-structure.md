# Repository Structure

This document explains how the repository is organized so that new contributors can quickly place code in the right layer.

## Top-Level Directories

- `src/`: application source code.
- `tests/`: unit tests for core logic and UI components.
- `config/`: runtime defaults, model templates, and operator JSON schemas.
- `samples/`: example pipelines, project descriptors, and placeholder asset folders.
- `resources/`: icons, resource collections, and Windows resource files.
- `docs/`: build instructions, review artifacts, design documents, and repository guidance.
  - [build-windows-mingw.md](build-windows-mingw.md), [build-linux.md](build-linux.md), [build-macos.md](build-macos.md)
  - [quick-start-validation.md](quick-start-validation.md)
  - [manual-validation-v0.1.0.md](manual-validation-v0.1.0.md)
- `qmake/`: shared qmake include files and explicit source lists.
- `packaging/`: release packaging helpers for Windows, Linux, and macOS (`package.ps1`, `package.sh`).
- `scripts/`: maintenance utilities such as operator schema and sample asset generation.
- `opencvsdk/`: bundled Windows MinGW OpenCV SDK tracked by the repository.

## Source Layout

- `src/app/`: startup configuration, service locator, command dispatch, and persisted application settings.
- `src/core/`: media sources, operators, detection, pipeline, project, task, and model domain logic.
- `src/infra/`: framework or vendor integration helpers such as OpenCV/Qt conversion utilities and platform paths.
- `src/ui/`: main window, dialogs, panels, and widgets.

Keep algorithmic processing in `src/core` whenever possible. Use `src/infra` for adapter code and `src/ui` for QWidget behavior and presentation logic.

## Configuration Layout

- `config/app_defaults.json`: documented default values for preview, export, and DNN settings; loaded at startup via `AppDefaultsLoader` before user `QSettings` overrides.
- `config/model_templates/`: ONNX model descriptor templates by task type.
- `config/operator_schemas/`: JSON parameter schemas for built-in operators; validated against runtime `StepSchema` definitions.

At runtime, `PlatformPaths::resolveResourcePath()` searches beside the executable and the current working directory for these folders. Documentation files resolve through `PlatformPaths::resolveDocumentationPath()`.

## Test Layout

- `tests/unit/core/`: unit tests for pipeline, media, registry, serialization, detection, and schema logic.
- `tests/unit/ui/`: unit tests for panels, dialogs, the main window, and canvas behavior.
- `tests/integration/`: smoke tests for cross-module wiring.

The aggregated executable name is `CVVerifyCoreTests` for both CMake and qmake workflows.

## Build Files

- `CMakeLists.txt`: primary CMake build entry point.
- `CVVerify.pro`: qmake application build entry point.
- `CVVerifyCoreTests.pro`: qmake test build entry point.
- `qmake/*.pri`: explicit source lists used by qmake builds.

CMake copies `config/` and `samples/` next to `CVVerify.exe` after each build.

## Documents Worth Reading First

- [Windows MinGW Build](build-windows-mingw.md)
- [Quick Start Validation](quick-start-validation.md)
- [Manual Validation Checklist (v0.1.0)](manual-validation-v0.1.0.md)
- [Open Source Release Checklist](open-source-release-checklist.md)
- [Design Spec](superpowers/specs/2026-06-04-opencv-validation-platform-design.md)
