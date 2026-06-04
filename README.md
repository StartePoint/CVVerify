# CVVerify

CVVerify is an open-source desktop validation platform for computer vision R&D.

It is designed for algorithm engineers who need to validate:

- traditional OpenCV image processing pipelines
- ONNX model inference behavior through OpenCV DNN
- image and video processing results under fully exposed parameters
- reproducible experiment configurations and exported outputs

## Vision

CVVerify aims to provide one workstation for:

- image experimentation
- video preview and offline validation
- model import and configuration
- operator and inference pipeline composition
- result comparison, diagnostics, and export

The first release targets Windows, while the codebase is structured for future cross-platform support.

## Planned Core Features

- Traditional image processing operators with schema-driven parameter panels
- User-provided ONNX model import and categorization
- Dual-entry model configuration:
  - import from training-platform outputs
  - generate/edit through a visual configuration editor
- Unified frame-stream abstraction for both images and videos
- Real-time preview and offline processing pipelines
- Overlay rendering, structured result export, and reproducible configuration snapshots

## Current Status

This repository is in the architecture and planning stage.

Available now:

- product specification document

Planned next:

- implementation plan
- project skeleton
- core pipeline framework

## Specification

- [Design Spec](docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md)

## Repository Roadmap

1. Establish project skeleton and build system
2. Implement frame-stream pipeline core
3. Add traditional operator registry and preview loop
4. Add ONNX model import and inference workflow
5. Add batch processing, offline video, and export features
6. Prepare Windows release and open-source collaboration workflow

## Open Source

This project is intended to be open source from the beginning.

Planned repository standards include:

- contribution guide
- code of conduct
- security policy
- issue templates
- pull request template
- CI workflow

## License

Planned: MIT License
