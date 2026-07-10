# Sample Model Assets

Place ONNX models and label files here for local inference validation. Large weights are intentionally **not** bundled in git.

## Expected Layout

```text
samples/models/
  my_detector.onnx
  my_detector.labels.txt
  my_classifier.onnx
  my_classifier.labels.txt
```

## Label File Format

One class name per line. Line index maps to class id (0-based).

See `labels.example.txt` for a starter detection label list.

## Import Workflow

1. Copy your ONNX file into this folder.
2. Copy or create a matching `.labels.txt` file.
3. In CVVerify, use **Model → Import ONNX Model**.
4. Pick the task category (detection, classification, segmentation, OCR, keypoint, custom tensor).
5. Apply a template from `config/model_templates/` when prompted, then refine settings in the model config editor.

## Templates

Task-specific defaults live in `config/model_templates/`:

| Template | Task |
|----------|------|
| `yolo_detection.json` | Object detection |
| `classification.json` | Image classification |
| `segmentation.json` | Semantic / mask output |
| `ocr.json` | Text detection / recognition |
| `keypoint.json` | Keypoint / pose style outputs |

Adjust input size, color order, and postprocess thresholds after import to match your exported model.
