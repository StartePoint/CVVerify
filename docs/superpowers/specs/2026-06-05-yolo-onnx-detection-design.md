# YOLO ONNX Detection Design

- Date: 2026-06-05
- Target Stage: Detection vertical slice after media/operator foundation
- Target Platform: Windows desktop, MinGW toolchain, cross-platform oriented architecture
- Tech Stack: C++17 + Qt Widgets + OpenCV + OpenCV DNN + JSON + CMake
- Scope: YOLO ONNX object detection with image and video preview/export

## 1. Purpose

This design defines the first DNN-powered validation workflow for CVVerify.

The goal is to add a practical and reproducible object-detection path that lets R&D users:

- import a YOLO detection ONNX model
- bind it to a required `labels.txt`
- preview detection results on images
- preview detection results on videos
- export overlay media plus structured detection results
- keep preprocessing and postprocessing parameters visible and adjustable

This is intentionally the first DNN vertical slice, not a fully generic model platform.

## 2. Scope

### 2.1 In Scope

- YOLO-family ONNX detection model import
- required `labels.txt` class mapping
- model package generation and persistence
- single-image detection preview
- batch image detection export
- video preview with detection overlays
- offline video export with detection overlays
- `json/csv` result export
- integration into the existing pipeline and parameter editing flow

### 2.2 Out Of Scope

- training, retraining, fine-tuning
- online model conversion
- segmentation, pose, OCR, or oriented box support
- custom script postprocessing
- full model marketplace or plugin system
- multi-model ensembles

## 3. Recommended Approach

Recommended route: `YOLO-first templated detection framework`.

Why:

- faster than a completely generic detection engine
- more future-proof than a YOLO-only hardcoded implementation
- keeps import and runtime logic structured for later expansion to non-YOLO detectors

The first template is YOLO detection only. The internal architecture should still speak in broader detection concepts.

## 4. Supported Model Boundary

V1 detection support is intentionally explicit:

- format: `ONNX`
- task type: `detection`
- family: `YOLO detect`
- labels: `labels.txt` required
- initial compatibility target: common `YOLOv5 / YOLOv8 / YOLOv11 detect` style exports

If the model does not match this boundary, import should fail with a clear explanation instead of pretending to support it.

## 5. Model Package Structure

Each imported model should be normalized into a package:

```text
models/
  yolo_demo/
    model.onnx
    labels.txt
    model.json
    preview.jpg
```

Notes:

- `model.onnx`: original ONNX file
- `labels.txt`: one class label per line
- `model.json`: unified internal configuration
- `preview.jpg`: optional preview or last-verified example

This keeps model management, project persistence, and export reproducibility aligned with the rest of CVVerify.

## 6. Internal YOLO Model Config

Example `model.json` shape:

```json
{
  "model_name": "yolo_demo",
  "task_type": "detection",
  "framework_family": "yolo",
  "model_format": "onnx",
  "labels_path": "labels.txt",
  "input": {
    "width": 640,
    "height": 640,
    "channels": 3,
    "color_order": "RGB",
    "letterbox": true,
    "keep_ratio": true,
    "scale": 0.0039215686,
    "mean": [0, 0, 0],
    "std": [1, 1, 1]
  },
  "postprocess": {
    "confidence_threshold": 0.25,
    "nms_iou_threshold": 0.45,
    "multi_label": false,
    "max_detections": 300
  },
  "output": {
    "layout_type": "yolo_detect_auto"
  }
}
```

Key principle:

- model file and interpretation rules must stay decoupled

This lets the same ONNX model be re-validated with different preprocess or postprocess settings later.

## 7. Import Workflow

Recommended import flow:

1. Select `model.onnx`
2. Select `labels.txt`
3. Read ONNX input metadata
4. Pre-fill YOLO detection template defaults
5. Let user review and adjust:
   - input size
   - color order
   - letterbox
   - confidence threshold
   - NMS IoU threshold
6. Save model package
7. Offer immediate preview on the currently loaded image

This workflow keeps common YOLO imports fast while still exposing the parameters researchers care about.

## 8. Runtime Data Flow

Detection processing flow:

`FramePacket -> preprocess -> blob -> OpenCV DNN inference -> YOLO decode -> threshold filter -> NMS -> render -> export`

Detailed breakdown:

1. Preserve `originalMat`
2. Resize / letterbox / color reorder / scale
3. Create `blob`
4. Run OpenCV DNN
5. Decode raw tensor into candidate detections
6. Apply confidence threshold
7. Apply NMS
8. Map boxes back into original-image coordinates
9. Render boxes and labels
10. Export structured output

Important runtime metadata to preserve:

- input width and height
- resize ratio
- padding offsets from letterbox
- confidence threshold
- NMS threshold
- per-frame inference time

## 9. Detection Result Structures

Recommended internal structures:

```cpp
struct DetectionBox
{
    int classId = -1;
    QString label;
    float score = 0.0f;
    QRectF box;
};

struct DetectionFrameResult
{
    qint64 frameId = -1;
    qint64 timestampMs = 0;
    QString sourceId;
    QVector<DetectionBox> boxes;
    QVariantMap runtimeMeta;
};
```

Conventions:

- boxes use original-image coordinates
- runtime metadata stores thresholds, timing, and preprocessing information

## 10. Preview And Export Workflow

### 10.1 Image Preview

Must support:

- import YOLO model package
- run detection on current image
- live tuning of:
  - confidence threshold
  - NMS threshold
  - input size
  - letterbox
  - color order

### 10.2 Batch Image Export

Must support:

- directory or multi-image input
- overlay image export
- per-image `json`
- aggregate `csv`

### 10.3 Video Preview

Must support:

- frame sampling preview
- playback-time overlay preview
- latest-frame-wins display policy

### 10.4 Offline Video Export

Must support:

- full-frame sequential processing
- overlay video export
- per-frame `json/csv`
- timing summary

## 11. Export Outputs

V1 export set:

- overlay image
- overlay video
- detection `json`
- detection `csv`
- pipeline snapshot
- model config snapshot

Recommended per-detection JSON fields:

- source file or frame index
- frame timestamp
- image width and height
- class id
- class label
- score
- `x`, `y`, `w`, `h`
- preprocessing and threshold metadata

## 12. Module Breakdown

Recommended units:

1. `Model Package Manager`
   - `DetectionModelDescriptor`
   - `ModelPackageLoader`
   - `LabelProvider`

2. `YOLO Importer`
   - `YoloOnnxImportService`
   - `YoloTemplateConfigBuilder`

3. `Detection Runtime`
   - `DetectionInferenceSession`
   - `OpenCvDnnRunner`

4. `YOLO Postprocess`
   - `YoloDetectionDecoder`
   - `DetectionNmsProcessor`

5. `Detection Render`
   - `DetectionRenderComposer`

6. `Detection Export`
   - `DetectionExportService`

7. `Application Control`
   - `DetectionPreviewController`
   - `DetectionTaskController`

## 13. Integration Direction

This detection stack should not be forced into the existing builtin-operator namespace.

Recommended integration:

- detection runtime and postprocess live under a new `core/dnn` or `core/detection` area
- detection preview enters the existing preview pipeline through a dedicated `IPipelineStep` implementation
- UI remains orchestration-only and does not decode tensors directly

## 14. Implementation Order

Recommended rollout:

1. YOLO model package import
2. Single-image detection preview
3. Batch image export
4. Video preview
5. Offline video export
6. Stability, diagnostics, and UX polish

## 15. Success Criteria

This vertical slice is successful when:

- a user can import `model.onnx + labels.txt`
- the model package is persisted correctly
- a current image can show detection overlays
- thresholds can be adjusted and immediately re-previewed
- image batches can export overlay and structured results
- videos can preview and export detections
- exported results are reproducible using saved pipeline and model snapshots
