# Manual Validation Checklist (v0.1.0)

Use this checklist on a clean Windows MinGW workstation before tagging the v0.1.0 pre-release.

## Build and Launch

- [ ] Configure and build with CMake or qmake per [build-windows-mingw.md](build-windows-mingw.md)
- [ ] Run `CVVerifyCoreTests` and confirm all unit/integration tests pass
- [ ] Launch the desktop app; confirm `config/` and `samples/` are beside the executable
- [ ] Confirm status bar shows version and DNN backend (for example `CVVerify 0.1.0 | cpu`)

## First-Run and Help

- [ ] First launch shows welcome guide (unless disabled in prior settings)
- [ ] **Help → Quick Start Guide** opens bundled documentation
- [ ] **Help → Manual Validation Checklist** opens this file
- [ ] **Help → About CVVerify** lists OpenCV/Qt versions and available DNN backends

## Sample Workflows

- [ ] **Project → Load Sample Project → Demo Blur Project** loads media and pipeline
- [ ] **Preview → Full Pipeline Preview** updates processed canvas
- [ ] Toggle **Dual View**, **Wipe Comparison**, and **Four Grid** preview modes
- [ ] **Preview → Capture Comparison Baseline** and compare after parameter changes
- [ ] **Pipeline → Load Sample Pipeline** loads a bundled JSON descriptor
- [ ] Drag-and-drop image, video, folder, pipeline JSON, project JSON, and `.onnx` model

## Settings and Persistence

- [ ] **Settings → System Settings** changes language, preview FPS, and DNN backend
- [ ] **Reset to Defaults** restores values from `config/app_defaults.json`
- [ ] **Clear Recent Files** removes media/project recent-menu entries after save
- [ ] **Max Recent Files** trims menus to the configured limit
- [ ] Default export directory hint appears when the field is empty
- [ ] Export with **Open output directory automatically after export** opens the output folder

## Export and Diagnostics

- [ ] **Export → Export Detection Images** writes overlay and JSON artifacts
- [ ] **Export → Export Pipeline Snapshot** and **Export App Config Snapshot** create JSON files
- [ ] Diagnostics panel shows startup schema validation and runtime backend entries
- [ ] Statistics panel updates after preview rerun

## Packaging Smoke Test

- [ ] Run `packaging/windows/package.ps1` on a release build tree
- [ ] Launch packaged `dist/CVVerify/CVVerify.exe` and repeat sample-project load

## Known Limitations to Verify Messaging

- [ ] SURF operators remain placeholders and are documented as such
- [ ] CUDA backend appears only when the linked OpenCV build supports it
- [ ] Unsupported drag-and-drop types show a localized status-bar message

## Sign-Off

| Field | Value |
| --- | --- |
| Tester | |
| Date | |
| Build commit | |
| Toolchain (Qt / MinGW / OpenCV) | |
| Result | Pass / Fail |
| Notes | |
