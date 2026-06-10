# CVVerify YOLO ONNX Detection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first DNN-powered validation workflow for CVVerify by importing YOLO detection ONNX models, previewing detections on images and videos, and exporting overlay plus structured results.

**Architecture:** Keep the existing modular monolith and add a dedicated `core/detection` path instead of forcing DNN logic into the builtin image-operator namespace. The UI continues to orchestrate workflow only, while model import, DNN runtime, YOLO decode, render, and export live behind focused services and a detection-specific pipeline step.

**Tech Stack:** C++17, Qt Widgets, Qt Test, OpenCV 4.12.0, OpenCV DNN, JSON, CMake, bundled OpenCV SDK, MinGW 7.3.0

---

## File Structure Map

- `CMakeLists.txt`
  - Add detection sources and tests
- `src/core/detection/DetectionTypes.h`
  - Detection box and frame-result DTOs
- `src/core/detection/DetectionModelDescriptor.h`
  - Persisted YOLO model package configuration
- `src/core/detection/ModelPackageLoader.h`
  - Model package read/write contract
- `src/core/detection/ModelPackageLoader.cpp`
  - Model package read/write implementation
- `src/core/detection/LabelProvider.h`
  - Labels file loading contract
- `src/core/detection/LabelProvider.cpp`
  - Labels file loading implementation
- `src/core/detection/YoloOnnxImportService.h`
  - YOLO import workflow contract
- `src/core/detection/YoloOnnxImportService.cpp`
  - YOLO import workflow implementation
- `src/core/detection/OpenCvDnnRunner.h`
  - OpenCV DNN inference wrapper
- `src/core/detection/OpenCvDnnRunner.cpp`
  - OpenCV DNN inference wrapper implementation
- `src/core/detection/YoloDetectionDecoder.h`
  - YOLO output decode and NMS contract
- `src/core/detection/YoloDetectionDecoder.cpp`
  - YOLO output decode and NMS implementation
- `src/core/detection/DetectionRenderComposer.h`
  - Detection overlay rendering contract
- `src/core/detection/DetectionRenderComposer.cpp`
  - Detection overlay rendering implementation
- `src/core/detection/DetectionExportService.h`
  - Batch image/video export contract
- `src/core/detection/DetectionExportService.cpp`
  - Batch image/video export implementation
- `src/core/detection/YoloDetectionStep.h`
  - Detection-specific `IPipelineStep`
- `src/core/detection/YoloDetectionStep.cpp`
  - Detection-specific `IPipelineStep` implementation
- `src/ui/dialogs/YoloModelImportDialog.h`
  - Import dialog declaration
- `src/ui/dialogs/YoloModelImportDialog.cpp`
  - Import dialog implementation
- `src/ui/dialogs/YoloModelImportDialog.ui`
  - Import dialog layout
- `src/ui/panels/MediaPanel.h`
  - Add batch image and video export actions
- `src/ui/panels/MediaPanel.cpp`
  - Add batch image and video export actions
- `src/ui/mainwindow/MainWindow.h`
  - Add detection workflow state and slots
- `src/ui/mainwindow/MainWindow.cpp`
  - Wire model import, detection preview, and export
- `tests/unit/core/test_model_package_loader.cpp`
  - Model package persistence coverage
- `tests/unit/core/test_yolo_decoder.cpp`
  - YOLO decode and NMS coverage
- `tests/unit/core/test_yolo_detection_step.cpp`
  - Detection step preview coverage
- `tests/unit/ui/test_yolo_model_import_dialog.cpp`
  - Import dialog behavior coverage

## Task 1: Add model package types, labels loading, and YOLO import persistence

**Files:**
- Modify: `CMakeLists.txt`
- Create: `src/core/detection/DetectionTypes.h`
- Create: `src/core/detection/DetectionModelDescriptor.h`
- Create: `src/core/detection/ModelPackageLoader.h`
- Create: `src/core/detection/ModelPackageLoader.cpp`
- Create: `src/core/detection/LabelProvider.h`
- Create: `src/core/detection/LabelProvider.cpp`
- Create: `src/core/detection/YoloOnnxImportService.h`
- Create: `src/core/detection/YoloOnnxImportService.cpp`
- Create: `tests/unit/core/test_model_package_loader.cpp`

- [ ] **Step 1: Write the failing test for model package save/load**

Create `tests/unit/core/test_model_package_loader.cpp`:

```cpp
#include <QtTest>

#include <QTemporaryDir>

#include "core/detection/ModelPackageLoader.h"

class ModelPackageLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void savesAndLoadsDetectionModelDescriptor();
};

void ModelPackageLoaderTest::savesAndLoadsDetectionModelDescriptor()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DetectionModelDescriptor descriptor;
    descriptor.modelName = "yolo_demo";
    descriptor.taskType = "detection";
    descriptor.frameworkFamily = "yolo";
    descriptor.modelFormat = "onnx";
    descriptor.labelsPath = "labels.txt";
    descriptor.input.width = 640;
    descriptor.input.height = 640;
    descriptor.postprocess.confidenceThreshold = 0.25f;
    descriptor.postprocess.nmsIouThreshold = 0.45f;

    QString errorMessage;
    QVERIFY(ModelPackageLoader::saveDescriptor(descriptor, dir.path(), &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    const DetectionModelLoadResult result = ModelPackageLoader::loadDescriptor(dir.path());

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.descriptor.modelName, QString("yolo_demo"));
    QCOMPARE(result.descriptor.input.width, 640);
    QCOMPARE(result.descriptor.input.height, 640);
    QCOMPARE(result.descriptor.postprocess.confidenceThreshold, 0.25f);
    QCOMPARE(result.descriptor.postprocess.nmsIouThreshold, 0.45f);
}

int runModelPackageLoaderTests(int argc, char* argv[])
{
    ModelPackageLoaderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_model_package_loader.moc"
```

- [ ] **Step 2: Wire the new test into the test runner and watch it fail**

Update `tests/unit/core/test_main.cpp`:

```cpp
int runModelPackageLoaderTests(int argc, char* argv[]);
```

and in `main(...)`:

```cpp
status |= runModelPackageLoaderTests(argc, argv);
```

Add to `CMakeLists.txt` test target:

```cmake
        tests/unit/core/test_model_package_loader.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
```

Expected: FAIL because detection package files do not exist yet.

- [ ] **Step 3: Add the minimal detection model descriptor and loader implementation**

Create `src/core/detection/DetectionTypes.h`:

```cpp
#pragma once

#include <QRectF>
#include <QString>
#include <QVector>
#include <QVariantMap>

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

Create `src/core/detection/DetectionModelDescriptor.h`:

```cpp
#pragma once

#include <QString>

struct DetectionInputConfig
{
    int width = 640;
    int height = 640;
    int channels = 3;
    QString colorOrder = "RGB";
    bool letterbox = true;
    bool keepRatio = true;
    float scale = 0.0039215686f;
};

struct DetectionPostprocessConfig
{
    float confidenceThreshold = 0.25f;
    float nmsIouThreshold = 0.45f;
    bool multiLabel = false;
    int maxDetections = 300;
};

struct DetectionOutputConfig
{
    QString layoutType = "yolo_detect_auto";
};

struct DetectionModelDescriptor
{
    QString modelName;
    QString taskType = "detection";
    QString frameworkFamily = "yolo";
    QString modelFormat = "onnx";
    QString labelsPath = "labels.txt";
    DetectionInputConfig input;
    DetectionPostprocessConfig postprocess;
    DetectionOutputConfig output;
};

struct DetectionModelLoadResult
{
    bool success = false;
    QString errorMessage;
    DetectionModelDescriptor descriptor;
};
```

Create `src/core/detection/ModelPackageLoader.h`:

```cpp
#pragma once

#include <QString>

#include "core/detection/DetectionModelDescriptor.h"

namespace ModelPackageLoader {

bool saveDescriptor(const DetectionModelDescriptor& descriptor, const QString& packageDir, QString* errorMessage = nullptr);
DetectionModelLoadResult loadDescriptor(const QString& packageDir);

}
```

Create `src/core/detection/ModelPackageLoader.cpp`:

```cpp
#include "core/detection/ModelPackageLoader.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QJsonObject toJson(const DetectionModelDescriptor& descriptor)
{
    return {
        {"model_name", descriptor.modelName},
        {"task_type", descriptor.taskType},
        {"framework_family", descriptor.frameworkFamily},
        {"model_format", descriptor.modelFormat},
        {"labels_path", descriptor.labelsPath},
        {"input", QJsonObject{
            {"width", descriptor.input.width},
            {"height", descriptor.input.height},
            {"channels", descriptor.input.channels},
            {"color_order", descriptor.input.colorOrder},
            {"letterbox", descriptor.input.letterbox},
            {"keep_ratio", descriptor.input.keepRatio},
            {"scale", descriptor.input.scale}
        }},
        {"postprocess", QJsonObject{
            {"confidence_threshold", descriptor.postprocess.confidenceThreshold},
            {"nms_iou_threshold", descriptor.postprocess.nmsIouThreshold},
            {"multi_label", descriptor.postprocess.multiLabel},
            {"max_detections", descriptor.postprocess.maxDetections}
        }},
        {"output", QJsonObject{
            {"layout_type", descriptor.output.layoutType}
        }}
    };
}

DetectionModelDescriptor fromJson(const QJsonObject& root)
{
    DetectionModelDescriptor descriptor;
    descriptor.modelName = root.value("model_name").toString();
    descriptor.taskType = root.value("task_type").toString("detection");
    descriptor.frameworkFamily = root.value("framework_family").toString("yolo");
    descriptor.modelFormat = root.value("model_format").toString("onnx");
    descriptor.labelsPath = root.value("labels_path").toString("labels.txt");

    const QJsonObject input = root.value("input").toObject();
    descriptor.input.width = input.value("width").toInt(640);
    descriptor.input.height = input.value("height").toInt(640);
    descriptor.input.channels = input.value("channels").toInt(3);
    descriptor.input.colorOrder = input.value("color_order").toString("RGB");
    descriptor.input.letterbox = input.value("letterbox").toBool(true);
    descriptor.input.keepRatio = input.value("keep_ratio").toBool(true);
    descriptor.input.scale = static_cast<float>(input.value("scale").toDouble(0.0039215686));

    const QJsonObject post = root.value("postprocess").toObject();
    descriptor.postprocess.confidenceThreshold = static_cast<float>(post.value("confidence_threshold").toDouble(0.25));
    descriptor.postprocess.nmsIouThreshold = static_cast<float>(post.value("nms_iou_threshold").toDouble(0.45));
    descriptor.postprocess.multiLabel = post.value("multi_label").toBool(false);
    descriptor.postprocess.maxDetections = post.value("max_detections").toInt(300);

    descriptor.output.layoutType = root.value("output").toObject().value("layout_type").toString("yolo_detect_auto");
    return descriptor;
}

}

namespace ModelPackageLoader {

bool saveDescriptor(const DetectionModelDescriptor& descriptor, const QString& packageDir, QString* errorMessage)
{
    QDir dir;
    if (!dir.mkpath(packageDir)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to create package directory: %1").arg(packageDir);
        }
        return false;
    }

    QFile file(QDir(packageDir).filePath("model.json"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open model.json for writing");
        }
        return false;
    }

    file.write(QJsonDocument(toJson(descriptor)).toJson(QJsonDocument::Indented));
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

DetectionModelLoadResult loadDescriptor(const QString& packageDir)
{
    DetectionModelLoadResult result;
    QFile file(QDir(packageDir).filePath("model.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = QString("Failed to open model.json");
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        result.errorMessage = QString("model.json is not a valid JSON object");
        return result;
    }

    result.descriptor = fromJson(document.object());
    result.success = true;
    return result;
}

}
```

- [ ] **Step 4: Add labels loading and YOLO import scaffolding**

Create `src/core/detection/LabelProvider.h`:

```cpp
#pragma once

#include <QString>
#include <QStringList>

struct LabelLoadResult
{
    bool success = false;
    QString errorMessage;
    QStringList labels;
};

namespace LabelProvider {

LabelLoadResult loadLabels(const QString& filePath);

}
```

Create `src/core/detection/LabelProvider.cpp`:

```cpp
#include "core/detection/LabelProvider.h"

#include <QFile>
#include <QTextStream>

namespace LabelProvider {

LabelLoadResult loadLabels(const QString& filePath)
{
    LabelLoadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QString("Failed to open labels file: %1").arg(filePath);
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (!line.isEmpty()) {
            result.labels.append(line);
        }
    }

    if (result.labels.isEmpty()) {
        result.errorMessage = QString("Labels file is empty: %1").arg(filePath);
        return result;
    }

    result.success = true;
    return result;
}

}
```

Create `src/core/detection/YoloOnnxImportService.h`:

```cpp
#pragma once

#include <QString>

#include "core/detection/DetectionModelDescriptor.h"

struct YoloImportRequest
{
    QString modelPath;
    QString labelsPath;
    QString modelName;
};

struct YoloImportResult
{
    bool success = false;
    QString errorMessage;
    DetectionModelDescriptor descriptor;
};

namespace YoloOnnxImportService {

YoloImportResult buildImportDescriptor(const YoloImportRequest& request);

}
```

Create `src/core/detection/YoloOnnxImportService.cpp`:

```cpp
#include "core/detection/YoloOnnxImportService.h"

#include <QFileInfo>

#include "core/detection/LabelProvider.h"

namespace YoloOnnxImportService {

YoloImportResult buildImportDescriptor(const YoloImportRequest& request)
{
    YoloImportResult result;

    if (!QFileInfo::exists(request.modelPath)) {
        result.errorMessage = QString("Model file does not exist: %1").arg(request.modelPath);
        return result;
    }

    const LabelLoadResult labels = LabelProvider::loadLabels(request.labelsPath);
    if (!labels.success) {
        result.errorMessage = labels.errorMessage;
        return result;
    }

    result.descriptor.modelName = request.modelName;
    result.descriptor.labelsPath = "labels.txt";
    result.success = true;
    return result;
}

}
```

- [ ] **Step 5: Run the test suite and verify green**

Add new sources to `CMakeLists.txt`:

For `CVVerify`:

```cmake
        src/core/detection/DetectionTypes.h
        src/core/detection/DetectionModelDescriptor.h
        src/core/detection/ModelPackageLoader.h
        src/core/detection/ModelPackageLoader.cpp
        src/core/detection/LabelProvider.h
        src/core/detection/LabelProvider.cpp
        src/core/detection/YoloOnnxImportService.h
        src/core/detection/YoloOnnxImportService.cpp
```

For `CVVerifyCoreTests`:

```cmake
        tests/unit/core/test_model_package_loader.cpp
        src/core/detection/ModelPackageLoader.cpp
        src/core/detection/LabelProvider.cpp
        src/core/detection/YoloOnnxImportService.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw -V
```

Expected:

- model package save/load test passes
- existing tests still pass

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt src/core/detection/DetectionTypes.h src/core/detection/DetectionModelDescriptor.h src/core/detection/ModelPackageLoader.h src/core/detection/ModelPackageLoader.cpp src/core/detection/LabelProvider.h src/core/detection/LabelProvider.cpp src/core/detection/YoloOnnxImportService.h src/core/detection/YoloOnnxImportService.cpp tests/unit/core/test_main.cpp tests/unit/core/test_model_package_loader.cpp
git commit -m "feat: add yolo model package import foundation"
```

## Task 2: Add YOLO output decode, NMS, and a detection pipeline step

**Files:**
- Create: `src/core/detection/OpenCvDnnRunner.h`
- Create: `src/core/detection/OpenCvDnnRunner.cpp`
- Create: `src/core/detection/YoloDetectionDecoder.h`
- Create: `src/core/detection/YoloDetectionDecoder.cpp`
- Create: `src/core/detection/YoloDetectionStep.h`
- Create: `src/core/detection/YoloDetectionStep.cpp`
- Create: `tests/unit/core/test_yolo_decoder.cpp`
- Create: `tests/unit/core/test_yolo_detection_step.cpp`
- Modify: `tests/unit/core/test_main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing decoder test**

Create `tests/unit/core/test_yolo_decoder.cpp`:

```cpp
#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/YoloDetectionDecoder.h"

class YoloDecoderTest : public QObject
{
    Q_OBJECT

private slots:
    void decodesSingleDetectionRow();
};

void YoloDecoderTest::decodesSingleDetectionRow()
{
    cv::Mat output(1, 6, CV_32FC1);
    output.at<float>(0, 0) = 0.5f;
    output.at<float>(0, 1) = 0.5f;
    output.at<float>(0, 2) = 0.25f;
    output.at<float>(0, 3) = 0.25f;
    output.at<float>(0, 4) = 0.90f;
    output.at<float>(0, 5) = 0.80f;

    YoloDecodeRequest request;
    request.output = output;
    request.labels = {"person"};
    request.originalWidth = 640;
    request.originalHeight = 640;
    request.confidenceThreshold = 0.25f;
    request.nmsIouThreshold = 0.45f;

    const YoloDecodeResult result = YoloDetectionDecoder::decode(request);

    QVERIFY(result.success);
    QCOMPARE(result.boxes.size(), 1);
    QCOMPARE(result.boxes.at(0).label, QString("person"));
    QVERIFY(result.boxes.at(0).score > 0.70f);
}

int runYoloDecoderTests(int argc, char* argv[])
{
    YoloDecoderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_decoder.moc"
```

- [ ] **Step 2: Add the failing detection-step preview test**

Create `tests/unit/core/test_yolo_detection_step.cpp`:

```cpp
#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/YoloDetectionStep.h"

class YoloDetectionStepTest : public QObject
{
    Q_OBJECT

private slots:
    void reportsMissingModelAsFailure();
};

void YoloDetectionStepTest::reportsMissingModelAsFailure()
{
    DetectionModelDescriptor descriptor;
    descriptor.modelName = "missing";

    YoloDetectionStep step(descriptor, "missing-model.onnx", {"person"});

    FramePacket frame;
    frame.originalMat = cv::Mat(16, 16, CV_8UC3, cv::Scalar(0, 0, 0));
    frame.workingMat = frame.originalMat.clone();

    const StepResult result = step.execute(frame, {});

    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
}

int runYoloDetectionStepTests(int argc, char* argv[])
{
    YoloDetectionStepTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_detection_step.moc"
```

- [ ] **Step 3: Wire new failing tests into the runner**

Update `tests/unit/core/test_main.cpp`:

```cpp
int runYoloDecoderTests(int argc, char* argv[]);
int runYoloDetectionStepTests(int argc, char* argv[]);
```

and:

```cpp
status |= runYoloDecoderTests(argc, argv);
status |= runYoloDetectionStepTests(argc, argv);
```

Add to `CMakeLists.txt` test target:

```cmake
        tests/unit/core/test_yolo_decoder.cpp
        tests/unit/core/test_yolo_detection_step.cpp
```

- [ ] **Step 4: Implement the minimal decoder and DNN runner**

Create `src/core/detection/OpenCvDnnRunner.h`:

```cpp
#pragma once

#include <QString>
#include <QVector>

#include <opencv2/core.hpp>

struct DnnRunResult
{
    bool success = false;
    QString errorMessage;
    QVector<cv::Mat> outputs;
};

namespace OpenCvDnnRunner {

DnnRunResult runSingleOutputModel(const QString& modelPath, const cv::Mat& blob);

}
```

Create `src/core/detection/OpenCvDnnRunner.cpp`:

```cpp
#include "core/detection/OpenCvDnnRunner.h"

#include <opencv2/dnn.hpp>

namespace OpenCvDnnRunner {

DnnRunResult runSingleOutputModel(const QString& modelPath, const cv::Mat& blob)
{
    DnnRunResult result;

    try {
        cv::dnn::Net net = cv::dnn::readNet(modelPath.toStdString());
        if (net.empty()) {
            result.errorMessage = QString("Failed to load model: %1").arg(modelPath);
            return result;
        }

        net.setInput(blob);
        result.outputs.append(net.forward());
        result.success = true;
        return result;
    } catch (const cv::Exception& e) {
        result.errorMessage = QString::fromStdString(e.what());
        return result;
    }
}

}
```

Create `src/core/detection/YoloDetectionDecoder.h`:

```cpp
#pragma once

#include <QStringList>

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"

struct YoloDecodeRequest
{
    cv::Mat output;
    QStringList labels;
    int originalWidth = 0;
    int originalHeight = 0;
    float confidenceThreshold = 0.25f;
    float nmsIouThreshold = 0.45f;
};

struct YoloDecodeResult
{
    bool success = false;
    QString errorMessage;
    QVector<DetectionBox> boxes;
};

namespace YoloDetectionDecoder {

YoloDecodeResult decode(const YoloDecodeRequest& request);

}
```

Create `src/core/detection/YoloDetectionDecoder.cpp`:

```cpp
#include "core/detection/YoloDetectionDecoder.h"

#include <opencv2/dnn.hpp>

namespace YoloDetectionDecoder {

YoloDecodeResult decode(const YoloDecodeRequest& request)
{
    YoloDecodeResult result;

    if (request.output.empty()) {
        result.errorMessage = "YOLO output tensor is empty";
        return result;
    }

    QVector<cv::Rect> boxes;
    QVector<float> scores;
    QVector<int> classIds;

    for (int row = 0; row < request.output.rows; ++row) {
        const float* data = request.output.ptr<float>(row);
        if (!data) {
            continue;
        }

        const float objectness = data[4];
        if (request.output.cols <= 5) {
            continue;
        }

        int bestClassId = 0;
        float bestClassScore = data[5];
        for (int col = 6; col < request.output.cols; ++col) {
            if (data[col] > bestClassScore) {
                bestClassScore = data[col];
                bestClassId = col - 5;
            }
        }

        const float finalScore = objectness * bestClassScore;
        if (finalScore < request.confidenceThreshold) {
            continue;
        }

        const float cx = data[0] * request.originalWidth;
        const float cy = data[1] * request.originalHeight;
        const float w = data[2] * request.originalWidth;
        const float h = data[3] * request.originalHeight;

        const int left = static_cast<int>(cx - w * 0.5f);
        const int top = static_cast<int>(cy - h * 0.5f);

        boxes.append(cv::Rect(left, top, static_cast<int>(w), static_cast<int>(h)));
        scores.append(finalScore);
        classIds.append(bestClassId);
    }

    std::vector<int> keep;
    cv::dnn::NMSBoxes(boxes.toStdVector(), scores.toStdVector(), request.confidenceThreshold, request.nmsIouThreshold, keep);

    for (int index : keep) {
        DetectionBox box;
        box.classId = classIds[index];
        box.label = request.labels.value(box.classId, QString("class_%1").arg(box.classId));
        box.score = scores[index];
        box.box = QRectF(boxes[index].x, boxes[index].y, boxes[index].width, boxes[index].height);
        result.boxes.append(box);
    }

    result.success = true;
    return result;
}

}
```

- [ ] **Step 5: Implement the minimal detection step**

Create `src/core/detection/YoloDetectionStep.h`:

```cpp
#pragma once

#include <QString>
#include <QStringList>

#include "core/detection/DetectionModelDescriptor.h"
#include "core/pipeline/IPipelineStep.h"

class YoloDetectionStep : public IPipelineStep
{
public:
    YoloDetectionStep(DetectionModelDescriptor descriptor, QString modelPath, QStringList labels);

    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    DetectionModelDescriptor m_descriptor;
    QString m_modelPath;
    QStringList m_labels;
};
```

Create `src/core/detection/YoloDetectionStep.cpp`:

```cpp
#include "core/detection/YoloDetectionStep.h"

#include <opencv2/dnn.hpp>

#include "core/detection/OpenCvDnnRunner.h"
#include "core/detection/YoloDetectionDecoder.h"

YoloDetectionStep::YoloDetectionStep(DetectionModelDescriptor descriptor, QString modelPath, QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString YoloDetectionStep::id() const
{
    return "detection.yolo";
}

QString YoloDetectionStep::displayName() const
{
    return "YOLO Detection";
}

StepSchema YoloDetectionStep::schema() const
{
    return {
        "detection.yolo",
        "YOLO Detection",
        {
            {"confidence_threshold", "Confidence Threshold", StepParameterType::Double, m_descriptor.postprocess.confidenceThreshold, 0.0, 1.0, {}},
            {"nms_iou_threshold", "NMS IoU Threshold", StepParameterType::Double, m_descriptor.postprocess.nmsIouThreshold, 0.0, 1.0, {}},
        }
    };
}

QVariantMap YoloDetectionStep::parameterValues() const
{
    return {
        {"confidence_threshold", m_descriptor.postprocess.confidenceThreshold},
        {"nms_iou_threshold", m_descriptor.postprocess.nmsIouThreshold},
    };
}

void YoloDetectionStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("confidence_threshold")) {
        m_descriptor.postprocess.confidenceThreshold = values.value("confidence_threshold").toFloat();
    }
    if (values.contains("nms_iou_threshold")) {
        m_descriptor.postprocess.nmsIouThreshold = values.value("nms_iou_threshold").toFloat();
    }
}

StepResult YoloDetectionStep::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (m_modelPath.isEmpty()) {
        return {false, "Model path is empty"};
    }

    cv::Mat blob = cv::dnn::blobFromImage(frame.workingMat, m_descriptor.input.scale, cv::Size(m_descriptor.input.width, m_descriptor.input.height));
    const DnnRunResult runResult = OpenCvDnnRunner::runSingleOutputModel(m_modelPath, blob);
    if (!runResult.success || runResult.outputs.isEmpty()) {
        return {false, runResult.errorMessage.isEmpty() ? QString("DNN run failed") : runResult.errorMessage};
    }

    const YoloDecodeResult decodeResult = YoloDetectionDecoder::decode({
        runResult.outputs.first().reshape(1, runResult.outputs.first().total() / runResult.outputs.first().size[runResult.outputs.first().dims - 1]),
        m_labels,
        frame.originalMat.cols,
        frame.originalMat.rows,
        m_descriptor.postprocess.confidenceThreshold,
        m_descriptor.postprocess.nmsIouThreshold
    });
    if (!decodeResult.success) {
        return {false, decodeResult.errorMessage};
    }

    frame.annotations.insert("detections", QVariant::fromValue(decodeResult.boxes.size()));
    return {};
}
```

- [ ] **Step 6: Run tests and verify green**

Add new files to `CMakeLists.txt`:

For `CVVerify`:

```cmake
        src/core/detection/OpenCvDnnRunner.h
        src/core/detection/OpenCvDnnRunner.cpp
        src/core/detection/YoloDetectionDecoder.h
        src/core/detection/YoloDetectionDecoder.cpp
        src/core/detection/YoloDetectionStep.h
        src/core/detection/YoloDetectionStep.cpp
```

For `CVVerifyCoreTests`:

```cmake
        tests/unit/core/test_yolo_decoder.cpp
        tests/unit/core/test_yolo_detection_step.cpp
        src/core/detection/OpenCvDnnRunner.cpp
        src/core/detection/YoloDetectionDecoder.cpp
        src/core/detection/YoloDetectionStep.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw -V
```

Expected:

- decoder tests pass
- missing-model detection-step test passes
- previous test suites still pass

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt src/core/detection/OpenCvDnnRunner.h src/core/detection/OpenCvDnnRunner.cpp src/core/detection/YoloDetectionDecoder.h src/core/detection/YoloDetectionDecoder.cpp src/core/detection/YoloDetectionStep.h src/core/detection/YoloDetectionStep.cpp tests/unit/core/test_main.cpp tests/unit/core/test_yolo_decoder.cpp tests/unit/core/test_yolo_detection_step.cpp
git commit -m "feat: add yolo detection runtime foundation"
```

## Task 3: Add a YOLO import dialog and model package import workflow

**Files:**
- Create: `src/ui/dialogs/YoloModelImportDialog.h`
- Create: `src/ui/dialogs/YoloModelImportDialog.cpp`
- Create: `src/ui/dialogs/YoloModelImportDialog.ui`
- Modify: `src/ui/panels/MediaPanel.h`
- Modify: `src/ui/panels/MediaPanel.cpp`
- Modify: `src/ui/mainwindow/MainWindow.h`
- Modify: `src/ui/mainwindow/MainWindow.cpp`
- Create: `tests/unit/ui/test_yolo_model_import_dialog.cpp`
- Modify: `tests/unit/core/test_main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing import-dialog test**

Create `tests/unit/ui/test_yolo_model_import_dialog.cpp`:

```cpp
#include <QtTest>

#include <QLineEdit>

#include "ui/dialogs/YoloModelImportDialog.h"

class YoloModelImportDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void buildsImportRequestFromFields();
};

void YoloModelImportDialogTest::buildsImportRequestFromFields()
{
    YoloModelImportDialog dialog;

    dialog.findChild<QLineEdit*>("modelPathEdit")->setText("model.onnx");
    dialog.findChild<QLineEdit*>("labelsPathEdit")->setText("labels.txt");
    dialog.findChild<QLineEdit*>("modelNameEdit")->setText("yolo_demo");

    const YoloImportRequest request = dialog.buildRequest();

    QCOMPARE(request.modelPath, QString("model.onnx"));
    QCOMPARE(request.labelsPath, QString("labels.txt"));
    QCOMPARE(request.modelName, QString("yolo_demo"));
}

int runYoloModelImportDialogTests(int argc, char* argv[])
{
    YoloModelImportDialogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_model_import_dialog.moc"
```

- [ ] **Step 2: Add the failing import workflow hook**

Update `tests/unit/core/test_main.cpp`:

```cpp
int runYoloModelImportDialogTests(int argc, char* argv[]);
```

and:

```cpp
status |= runYoloModelImportDialogTests(argc, argv);
```

Add test source in `CMakeLists.txt`:

```cmake
        tests/unit/ui/test_yolo_model_import_dialog.cpp
```

- [ ] **Step 3: Implement the import dialog**

Create `src/ui/dialogs/YoloModelImportDialog.h`:

```cpp
#pragma once

#include <QDialog>

#include "core/detection/YoloOnnxImportService.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class YoloModelImportDialog;
}
QT_END_NAMESPACE

class YoloModelImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit YoloModelImportDialog(QWidget* parent = nullptr);
    ~YoloModelImportDialog() override;

    YoloImportRequest buildRequest() const;

private:
    Ui::YoloModelImportDialog* ui;
};
```

Create `src/ui/dialogs/YoloModelImportDialog.cpp`:

```cpp
#include "ui/dialogs/YoloModelImportDialog.h"

#include "ui_YoloModelImportDialog.h"

YoloModelImportDialog::YoloModelImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::YoloModelImportDialog)
{
    ui->setupUi(this);
}

YoloModelImportDialog::~YoloModelImportDialog()
{
    delete ui;
}

YoloImportRequest YoloModelImportDialog::buildRequest() const
{
    return {
        ui->modelPathEdit->text(),
        ui->labelsPathEdit->text(),
        ui->modelNameEdit->text()
    };
}
```

Create `src/ui/dialogs/YoloModelImportDialog.ui`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>YoloModelImportDialog</class>
 <widget class="QDialog" name="YoloModelImportDialog">
  <property name="windowTitle">
   <string>Import YOLO ONNX Model</string>
  </property>
  <layout class="QFormLayout" name="formLayout">
   <item row="0" column="0"><widget class="QLabel" name="modelPathLabel"><property name="text"><string>Model Path</string></property></widget></item>
   <item row="0" column="1"><widget class="QLineEdit" name="modelPathEdit"/></item>
   <item row="1" column="0"><widget class="QLabel" name="labelsPathLabel"><property name="text"><string>Labels Path</string></property></widget></item>
   <item row="1" column="1"><widget class="QLineEdit" name="labelsPathEdit"/></item>
   <item row="2" column="0"><widget class="QLabel" name="modelNameLabel"><property name="text"><string>Model Name</string></property></widget></item>
   <item row="2" column="1"><widget class="QLineEdit" name="modelNameEdit"/></item>
  </layout>
 </widget>
 <resources/>
 <connections/>
</ui>
```

- [ ] **Step 4: Expose import action in the existing UI**

Update `src/ui/panels/MediaPanel.h`:

```cpp
signals:
    void importYoloModelRequested();
```

Add a new button declaration:

```cpp
QPushButton* m_importYoloModelButton = nullptr;
```

Update `src/ui/panels/MediaPanel.cpp` to create:

```cpp
m_importYoloModelButton = new QPushButton("Import YOLO Model", this);
connect(m_importYoloModelButton, &QPushButton::clicked, this, &MediaPanel::importYoloModelRequested);
layout->addWidget(m_importYoloModelButton);
```

- [ ] **Step 5: Wire import into `MainWindow`**

Update `src/ui/mainwindow/MainWindow.h`:

```cpp
void importYoloModel();
```

Add members:

```cpp
DetectionModelDescriptor m_activeDetectionModel;
QString m_activeDetectionModelPath;
QStringList m_activeDetectionLabels;
```

In `MainWindow.cpp`, connect:

```cpp
connect(m_mediaPanel, &MediaPanel::importYoloModelRequested, this, [this] { importYoloModel(); });
```

Then implement:

```cpp
void MainWindow::importYoloModel()
{
    YoloModelImportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const YoloImportRequest request = dialog.buildRequest();
    const YoloImportResult importResult = YoloOnnxImportService::buildImportDescriptor(request);
    if (!importResult.success) {
        statusBar()->showMessage(importResult.errorMessage);
        return;
    }

    const QString packageDir = QDir("models").filePath(request.modelName);
    QString errorMessage;
    if (!ModelPackageLoader::saveDescriptor(importResult.descriptor, packageDir, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    QFile::copy(request.modelPath, QDir(packageDir).filePath("model.onnx"));
    QFile::copy(request.labelsPath, QDir(packageDir).filePath("labels.txt"));

    const LabelLoadResult labels = LabelProvider::loadLabels(request.labelsPath);
    if (!labels.success) {
        statusBar()->showMessage(labels.errorMessage);
        return;
    }

    m_activeDetectionModel = importResult.descriptor;
    m_activeDetectionModelPath = QDir(packageDir).filePath("model.onnx");
    m_activeDetectionLabels = labels.labels;
    statusBar()->showMessage(QString("Imported YOLO model: %1").arg(request.modelName));
}
```

- [ ] **Step 6: Run tests and verify green**

Add new sources to `CMakeLists.txt`:

For `CVVerify`:

```cmake
        src/ui/dialogs/YoloModelImportDialog.h
        src/ui/dialogs/YoloModelImportDialog.cpp
        src/ui/dialogs/YoloModelImportDialog.ui
```

For `CVVerifyCoreTests`:

```cmake
        tests/unit/ui/test_yolo_model_import_dialog.cpp
        src/ui/dialogs/YoloModelImportDialog.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw -V
```

Expected:

- import dialog tests pass
- previous tests remain green

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt src/ui/dialogs/YoloModelImportDialog.h src/ui/dialogs/YoloModelImportDialog.cpp src/ui/dialogs/YoloModelImportDialog.ui src/ui/panels/MediaPanel.h src/ui/panels/MediaPanel.cpp src/ui/mainwindow/MainWindow.h src/ui/mainwindow/MainWindow.cpp tests/unit/core/test_main.cpp tests/unit/ui/test_yolo_model_import_dialog.cpp
git commit -m "feat: add yolo model import workflow"
```

## Task 4: Add single-image detection preview and overlay rendering

**Files:**
- Create: `src/core/detection/DetectionRenderComposer.h`
- Create: `src/core/detection/DetectionRenderComposer.cpp`
- Modify: `src/ui/mainwindow/MainWindow.cpp`
- Modify: `src/ui/mainwindow/MainWindow.h`
- Create: `tests/unit/core/test_detection_render_composer.cpp`
- Modify: `tests/unit/core/test_main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing render test**

Create `tests/unit/core/test_detection_render_composer.cpp`:

```cpp
#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/DetectionRenderComposer.h"

class DetectionRenderComposerTest : public QObject
{
    Q_OBJECT

private slots:
    void drawsOverlayForDetections();
};

void DetectionRenderComposerTest::drawsOverlayForDetections()
{
    cv::Mat image(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    result.boxes.append({0, "person", 0.88f, QRectF(10, 10, 20, 20)});

    DetectionRenderComposer::drawDetections(image, result);

    QVERIFY(image.at<cv::Vec3b>(10, 10) != cv::Vec3b(0, 0, 0));
}

int runDetectionRenderComposerTests(int argc, char* argv[])
{
    DetectionRenderComposerTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_detection_render_composer.moc"
```

- [ ] **Step 2: Wire the failing render test into the suite**

Update `tests/unit/core/test_main.cpp`:

```cpp
int runDetectionRenderComposerTests(int argc, char* argv[]);
```

and:

```cpp
status |= runDetectionRenderComposerTests(argc, argv);
```

Add the test source in `CMakeLists.txt`.

- [ ] **Step 3: Implement the render composer**

Create `src/core/detection/DetectionRenderComposer.h`:

```cpp
#pragma once

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result);

}
```

Create `src/core/detection/DetectionRenderComposer.cpp`:

```cpp
#include "core/detection/DetectionRenderComposer.h"

#include <opencv2/imgproc.hpp>

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result)
{
    for (const DetectionBox& box : result.boxes) {
        const cv::Rect rect(
            static_cast<int>(box.box.x()),
            static_cast<int>(box.box.y()),
            static_cast<int>(box.box.width()),
            static_cast<int>(box.box.height()));

        cv::rectangle(image, rect, cv::Scalar(0, 255, 0), 2);
        const std::string text = QString("%1 %.2f").arg(box.label).arg(box.score).toStdString();
        cv::putText(image, text, cv::Point(rect.x, std::max(10, rect.y - 4)), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }
}

}
```

- [ ] **Step 4: Wire single-image detection preview into `MainWindow`**

Update `MainWindow.h` to add:

```cpp
void previewDetectionOnCurrentImage();
```

Then in `MainWindow.cpp`, after successful import set up a detection step and rerun preview:

```cpp
void MainWindow::previewDetectionOnCurrentImage()
{
    if (!m_currentFrame.hasOriginalFrame() || m_activeDetectionModelPath.isEmpty()) {
        return;
    }

    auto detectionStep = std::make_shared<YoloDetectionStep>(m_activeDetectionModel, m_activeDetectionModelPath, m_activeDetectionLabels);

    FramePacket preview = m_currentFrame;
    preview.workingMat = preview.originalMat.clone();

    StepResult result = detectionStep->execute(preview, {});
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    DetectionFrameResult detections;
    const QVariant detectionCount = preview.annotations.value("detections");
    if (detectionCount.isValid()) {
        statusBar()->showMessage(QString("Detection preview complete: %1 boxes").arg(detectionCount.toInt()));
    }

    m_canvasView->setImage(OpenCvQtImageConverter::toQImage(preview.workingMat));
}
```

This first iteration may keep the actual detection metadata attached through annotations or a temporary adapter as long as the preview overlay appears and the flow is correct.

- [ ] **Step 5: Run tests and build the application**

Add the render-composer sources to `CMakeLists.txt`:

For `CVVerify`:

```cmake
        src/core/detection/DetectionRenderComposer.h
        src/core/detection/DetectionRenderComposer.cpp
```

For `CVVerifyCoreTests`:

```cmake
        tests/unit/core/test_detection_render_composer.cpp
        src/core/detection/DetectionRenderComposer.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw -V
cmake --build build-mingw --target CVVerify -- -j4
```

Expected:

- render tests pass
- previous tests still pass
- application builds with detection preview support

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt src/core/detection/DetectionRenderComposer.h src/core/detection/DetectionRenderComposer.cpp src/ui/mainwindow/MainWindow.h src/ui/mainwindow/MainWindow.cpp tests/unit/core/test_main.cpp tests/unit/core/test_detection_render_composer.cpp
git commit -m "feat: add yolo detection preview overlay"
```

## Task 5: Add export scaffolding for image and video detection results

**Files:**
- Create: `src/core/detection/DetectionExportService.h`
- Create: `src/core/detection/DetectionExportService.cpp`
- Modify: `src/ui/panels/MediaPanel.h`
- Modify: `src/ui/panels/MediaPanel.cpp`
- Modify: `src/ui/mainwindow/MainWindow.h`
- Modify: `src/ui/mainwindow/MainWindow.cpp`
- Create: `tests/unit/core/test_detection_export_service.cpp`
- Modify: `tests/unit/core/test_main.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing detection export test**

Create `tests/unit/core/test_detection_export_service.cpp`:

```cpp
#include <QtTest>

#include <QTemporaryDir>

#include <opencv2/core.hpp>

#include "core/detection/DetectionExportService.h"

class DetectionExportServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void exportsOverlayImageAndJson();
};

void DetectionExportServiceTest::exportsOverlayImageAndJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    cv::Mat image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    result.sourceId = "sample.png";
    result.boxes.append({0, "person", 0.91f, QRectF(4, 4, 8, 8)});

    QString errorMessage;
    QVERIFY(DetectionExportService::exportImageResult(image, result, dir.path(), &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("sample_overlay.png")));
    QVERIFY(QFileInfo::exists(dir.filePath("sample.json")));
}

int runDetectionExportServiceTests(int argc, char* argv[])
{
    DetectionExportServiceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_detection_export_service.moc"
```

- [ ] **Step 2: Implement the minimal export service**

Create `src/core/detection/DetectionExportService.h`:

```cpp
#pragma once

#include <QString>

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"

namespace DetectionExportService {

bool exportImageResult(const cv::Mat& image, const DetectionFrameResult& result, const QString& outputDir, QString* errorMessage = nullptr);

}
```

Create `src/core/detection/DetectionExportService.cpp`:

```cpp
#include "core/detection/DetectionExportService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <opencv2/imgcodecs.hpp>

#include "core/detection/DetectionRenderComposer.h"

namespace DetectionExportService {

bool exportImageResult(const cv::Mat& image, const DetectionFrameResult& result, const QString& outputDir, QString* errorMessage)
{
    QDir dir;
    if (!dir.mkpath(outputDir)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to create export directory: %1").arg(outputDir);
        }
        return false;
    }

    const QFileInfo fileInfo(result.sourceId);
    const QString stem = fileInfo.completeBaseName().isEmpty() ? "result" : fileInfo.completeBaseName();

    cv::Mat overlay = image.clone();
    DetectionRenderComposer::drawDetections(overlay, result);
    if (!cv::imwrite(QDir(outputDir).filePath(stem + "_overlay.png").toStdString(), overlay)) {
        if (errorMessage) {
            *errorMessage = "Failed to write overlay image";
        }
        return false;
    }

    QJsonObject root;
    root.insert("source_id", result.sourceId);
    QJsonArray boxes;
    for (const DetectionBox& box : result.boxes) {
        boxes.append(QJsonObject{
            {"class_id", box.classId},
            {"label", box.label},
            {"score", box.score},
            {"x", box.box.x()},
            {"y", box.box.y()},
            {"w", box.box.width()},
            {"h", box.box.height()}
        });
    }
    root.insert("boxes", boxes);

    QFile jsonFile(QDir(outputDir).filePath(stem + ".json"));
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to open JSON output file";
        }
        return false;
    }

    jsonFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

}
```

- [ ] **Step 3: Expose export actions in `MediaPanel`**

Update `MediaPanel.h`:

```cpp
signals:
    void exportDetectionImagesRequested();
    void exportDetectionVideoRequested();
```

Add buttons:

```cpp
QPushButton* m_exportDetectionImagesButton = nullptr;
QPushButton* m_exportDetectionVideoButton = nullptr;
```

Update `MediaPanel.cpp` to create and connect those buttons.

- [ ] **Step 4: Wire export entrypoints into `MainWindow`**

Update `MainWindow.h`:

```cpp
void exportDetectionImages();
void exportDetectionVideo();
```

Add corresponding `connect(...)` calls in `MainWindow.cpp`.

The first export version may limit actual behavior to image export and show a status message for video export if the video processing pipeline is not yet fully wired.

- [ ] **Step 5: Run tests and rebuild**

Add new sources to `CMakeLists.txt`:

For `CVVerify`:

```cmake
        src/core/detection/DetectionExportService.h
        src/core/detection/DetectionExportService.cpp
```

For `CVVerifyCoreTests`:

```cmake
        tests/unit/core/test_detection_export_service.cpp
        src/core/detection/DetectionExportService.cpp
        src/core/detection/DetectionRenderComposer.cpp
```

Run:

```powershell
$env:PATH="$PWD/opencvsdk/windows/opencv4.12/x64/mingw/bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;$env:PATH"
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw -V
cmake --build build-mingw --target CVVerify -- -j4
```

Expected:

- export service tests pass
- previous tests still pass
- application builds with detection export hooks

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt src/core/detection/DetectionExportService.h src/core/detection/DetectionExportService.cpp src/ui/panels/MediaPanel.h src/ui/panels/MediaPanel.cpp src/ui/mainwindow/MainWindow.h src/ui/mainwindow/MainWindow.cpp tests/unit/core/test_main.cpp tests/unit/core/test_detection_export_service.cpp
git commit -m "feat: add detection export scaffolding"
```

## Self-Review

### Spec coverage

- YOLO model package import: Task 1
- YOLO decode and DNN runtime: Task 2
- import dialog and package workflow: Task 3
- single-image detection preview: Task 4
- export scaffolding: Task 5
- image/video/batch extension points preserved: Tasks 4 and 5

### Placeholder scan

- No TODO/TBD placeholders remain in executable tasks
- Every task includes concrete file paths
- Every task includes concrete commands and expected outcomes

### Type consistency

- `DetectionModelDescriptor`, `DetectionFrameResult`, `YoloDecodeRequest`, `YoloDetectionStep`, and `DetectionExportService` names are consistent across tasks
- existing `IPipelineStep` integration is preserved

## Notes

This plan intentionally stops after the first practical YOLO detection vertical slice. Full video-export implementation details, richer model management, and generic detection support should be planned after this slice is stable.
