# CVVerify Sample Assets

This folder contains starter assets for local validation workflows.

## Layout

- `pipelines/` — example pipeline JSON files you can load from **Pipeline → Load Pipeline**
- `projects/` — example project descriptors
- `images/` — bundled test images (`checkerboard.png`, `gradient_ramp.png`) plus optional local assets
- `videos/` — place short test clips here
- `models/` — place ONNX models and label files here

## Quick Start

1. Optional: regenerate bundled images with `python scripts/generate_sample_assets.py`
2. Launch CVVerify and open `samples/images/checkerboard.png`, or use **Project → Load Sample Project**
3. Load a pipeline from **Pipeline → Load Sample Pipeline**, or use **Pipeline → Load Pipeline** for custom JSON
4. Run full pipeline preview

## Example Pipelines

| File | Purpose |
|------|---------|
| `pipelines/basic_blur.json` | Single Gaussian blur step |
| `pipelines/edge_canny.json` | Grayscale + Canny edge detection |
| `pipelines/morphology_open.json` | Threshold + morphology open |
| `pipelines/histogram_sidecar.json` | RGB histogram rendered as sidecar chart |

Example projects:

| File | Purpose |
|------|---------|
| `projects/demo_project.json` | Gaussian blur on checkerboard image |
| `projects/edge_canny_project.json` | Canny edges on gradient ramp image |

Model import guidance: [models/README.md](models/README.md)

## Notes

- Paths inside sample JSON use relative names; load media from this repository root or adjust paths after copying files.
- Model templates live in `config/model_templates/` at the repository root.
