# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog.

## [Unreleased]

### Added

- Initial architecture specification and implementation plans.
- Qt Widgets application shell with dual preview panels.
- Bundled OpenCV 4.12 Windows MinGW SDK integration.
- Schema-driven operator registry and embedded parameter editing.
- Traditional operator coverage across filtering, thresholding, edge detection, morphology, effects, detection, and segmentation categories.
- YOLO-oriented model packaging and detection export scaffolding.
- Core and UI unit test aggregation through `CVVerifyCoreTests`.
- Public repository support files for contribution, security, conduct, issue templates, and pull requests.

### Changed

- The embedded operator workbench can now be shown or hidden from the Pipeline menu to leave more space for preview panels.
- The main window default size is now `1920x1080`.
- Operator additions no longer depend on pop-up parameter dialogs for the main in-window workflow.
- Repository documentation now reflects the Windows-first toolchain, bundled dependency policy, and open-source publishing checklist.

### Documentation

- Expanded `README.md` with current capability coverage, build entry points, and repository layout.
- Added `THIRD_PARTY_NOTICES.md`.
- Added repository structure and open-source release checklist documents under `docs/`.
- Updated the Windows MinGW build guide to remove machine-specific repository paths.
