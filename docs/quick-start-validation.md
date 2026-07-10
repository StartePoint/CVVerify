# Quick Start: Validation Workflows

This guide walks through common CVVerify V1 validation tasks using bundled sample assets.

## 1. Traditional Operator Preview

1. Build and launch CVVerify (see [build-windows-mingw.md](build-windows-mingw.md)).
2. Use **Project → Load Sample Project → Demo Blur Project** to load bundled media and pipeline in one step.
3. Or open `samples/images/checkerboard.png` manually and use **Pipeline → Load Sample Pipeline**.
4. Run **Preview → Full Pipeline Preview** and inspect the canvas output.
5. Toggle **Preview → Dual View** or **Wipe Comparison** to compare against the source frame.

## 2. Edge and Morphology Chains

Load one of the bundled multi-step pipelines:

| Pipeline | Steps |
|----------|-------|
| `samples/pipelines/edge_canny.json` | Grayscale → Canny |
| `samples/pipelines/morphology_open.json` | Grayscale → Threshold → Morphology Open |
| `samples/pipelines/histogram_sidecar.json` | Histogram bar chart in sidecar mode |

Adjust parameters in the embedded operator workbench and re-run preview.

## 3. Comparison Baseline (A/B)

1. Run a pipeline on an image.
2. Use **Preview → Capture Comparison Baseline**.
3. Change operator parameters or swap a pipeline step.
4. Enable **Wipe** or **Four Grid** preview to compare baseline vs current output.
5. Use **Preview → Clear Comparison Baseline** when finished.

## 4. Project Save and Restore

1. Open a sample image and configure a pipeline.
2. Use **File → Save Project** (or load `samples/projects/demo_project.json`).
3. Close and reopen the project to verify reproducible media path, pipeline, and app settings.

## 5. ONNX Model Validation

1. Place an ONNX model and label file under `samples/models/` (see [samples/models/README.md](../samples/models/README.md)).
2. **Model → Import ONNX Model** — select task category and template.
3. Open **Model → Edit Active Model Config** to tune input size, thresholds, and preview.
4. Run inference on the current frame and inspect overlays.
5. Export structured results via **Export** menu (JSON, CSV, annotated images, or video segments).

## 6. Batch and Offline Tasks

1. Open an image folder as the media source.
2. Configure the pipeline and use batch export actions from the **Export** menu.
3. Monitor progress in the task console and task history panels.
4. Review diagnostics output for timing and failure records.

## 7. Drag-and-Drop and Snapshots

1. Drag a pipeline `.json`, project descriptor, or `.onnx` model onto the main window to load/import it directly.
2. Unsupported file types show a status-bar message instead of failing silently.
3. Use **Export → Export Pipeline Snapshot** to save the current pipeline JSON for reproducibility.
3. Use **Export → Export App Config Snapshot** to export runtime settings, recent files, and DNN preferences.
4. On first launch, the welcome dialog offers **Load Demo Project** or **Open Quick Start**.
5. Re-open the guide anytime via **Help → Show Welcome Guide**, or re-enable it in **Settings → Show welcome guide on next launch**.
6. Use **Help → Manual Validation Checklist** before tagging a pre-release build.
7. Use **Help → About CVVerify** to inspect runtime versions and available DNN backends.
8. The status bar shows the application version and active DNN backend.

## 8. Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+O` | Open Image |
| `Ctrl+Shift+O` | Load Project |
| `Ctrl+S` | Save Project |
| `Ctrl+Alt+S` | Save Pipeline |
| `F5` | Rerun Preview |
| `Space` | Play/Pause video or folder playback |
| `Ctrl+,` | System Settings |
| `Ctrl+Shift+D` | Show Diagnostics |

Timeline labels and frame counters follow the active interface language when scrubbing video or image-folder sequences.

## 9. Diagnostics and Schemas

- Open the diagnostics panel for runtime log entries.
- Operator parameter schemas live in `config/operator_schemas/` and are validated against runtime definitions in unit tests.
- Regenerate schemas after C++ parameter changes: `python scripts/generate_operator_schemas.py`

## Known Limitations

- SURF keypoint/match operators are placeholders pending upstream availability and licensing review.
- CUDA DNN backend selection requires an OpenCV build with CUDA support.
- Linux and macOS packaging scripts are provided; the primary validated release path is Windows MinGW.
