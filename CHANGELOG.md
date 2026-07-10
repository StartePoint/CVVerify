# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog.

## [Unreleased]

### Added

- Multi-type ONNX import and inference routing (detection, classification, segmentation, OCR, keypoint, custom tensor).
- `OnnxModelStepFactory` with dedicated steps for classification, segmentation, OCR, keypoint, and custom tensor outputs.
- Model configuration editor with six tabs, template save/load, and live preview.
- Generic ONNX import dialog with model category and template selection.
- Task history panel and task recording in the main window.
- Preview comparison baseline capture/clear for A/B validation workflows.
- Dual, wipe, and four-grid preview modes via `PreviewCanvasHost`.
- Histogram operator sidecar chart mode (`FramePacket::sidecarMat`).
- Tensor summary export (`*_tensor_summary.json`) and optional side-by-side detection video export.
- DNN backend selection (CPU / OpenCL / CUDA) in system settings and `OpenCvDnnRunner`.
- Operator JSON schemas under `config/operator_schemas/` with `OperatorSchemaLoader` validation and full coverage for all 48 built-in operators.
- `OperatorSchemaExporter` and `scripts/generate_operator_schemas.py` for schema regeneration from C++ sources.
- Model templates for classification, segmentation, OCR, and keypoint under `config/model_templates/`.
- Sample pipeline and project descriptors under `samples/`.
- Bundled sample images (`checkerboard.png`, `gradient_ramp.png`) and `scripts/generate_sample_assets.py`.
- Linux and macOS packaging scripts under `packaging/linux/` and `packaging/macos/`.
- Windows packaging script `packaging/windows/package.ps1`.
- GitHub Actions MinGW build and test job alongside repository metadata checks.
- CMake post-build copy of `config/` and `samples/` beside the executable.
- `AppDefaultsLoader` reads `config/app_defaults.json` as baseline settings before user preferences.
- Startup validation of indexed operator schemas logged to diagnostics.

### Changed

- `AppSettingsStore::load()` now applies `config/app_defaults.json` before user-specific `QSettings` overrides.
- `ApplicationContext` validates indexed operator schemas at startup and logs warnings on mismatch.
- Drag-and-drop support for pipeline and project JSON descriptors.
- Drag-and-drop unsupported file types now show a localized status-bar message.
- Drag-and-drop `.onnx` files opens the import dialog with the model path prefilled.
- Help menu entries to open bundled documentation (`quick-start-validation.md`, `build-windows-mingw.md`) and an About dialog.
- **Export → Export Pipeline Snapshot** for reproducible pipeline JSON export.
- `AppVersion` metadata helper and startup DNN runtime logging in `ApplicationContext`.
- Keyboard shortcuts for media/project actions, preview rerun, playback, settings, and diagnostics.
- **Project → Open Recent Project** menu with persisted history in `QSettings`.
- `DiagnosticsPanel` and `StatisticsPanel` localization and localized dock tab titles.
- First-run welcome guide with bundled demo project and quick-start shortcuts.
- **Help → Show Welcome Guide** and settings toggle to re-enable the welcome dialog on next launch.
- **Export → Export App Config Snapshot** for settings and recent-file reproducibility.
- Status bar version and DNN backend summary.
- **Help → Repository Structure** documentation entry.
- `TimelineWidget` localization for idle and frame counter labels.
- **Settings → Reset to Defaults** reloads bundled `config/app_defaults.json` values in the settings dialog.
- **Settings → Max Recent Files** controls media and project recent-menu capacity.
- **Settings → Clear Recent Files** removes persisted recent media and project entries.
- Default export directory hint explains fallback to source/current directory when left empty.
- **Help → Manual Validation Checklist** opens the v0.1.0 pre-release sign-off guide.
- `CommandDispatcher` unit tests plus `settings.open` and `diagnostics.show` commands.
- Main window integrates model registry, diagnostics, statistics, timeline, and extended export menus.
- `DetectionExportService` exports classification/tensor artifacts and supports video segment ranges.
- `ParameterPanel` supports image file pickers for template match paths.
- `ModelConfigEditorDialog` supports English and Simplified Chinese labels.
- `README.md` and repository structure docs updated for V1 workstation scope.

### Fixed

- OCR overlay text drawing uses a mutable baseline for OpenCV `getTextSize` compatibility.
- Saving system settings no longer clears persisted recent media and project file lists.

### Documentation

- Quick-start validation guide at `docs/quick-start-validation.md`.
- Linux and macOS build notes at `docs/build-linux.md` and `docs/build-macos.md`.
- Maintenance scripts documented in `scripts/README.md`.
- Sample model import guidance in `samples/models/README.md`.
- Expanded `README.md` with current capability coverage, packaging, and CI notes.
- Added operator schema README and sample asset guide.
- Updated `config/app_defaults.json` with preview, DNN, and export defaults.

## [0.1.0] - pre-release baseline

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
