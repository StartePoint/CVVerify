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

    const QJsonObject postprocess = root.value("postprocess").toObject();
    descriptor.postprocess.confidenceThreshold = static_cast<float>(postprocess.value("confidence_threshold").toDouble(0.25));
    descriptor.postprocess.nmsIouThreshold = static_cast<float>(postprocess.value("nms_iou_threshold").toDouble(0.45));
    descriptor.postprocess.multiLabel = postprocess.value("multi_label").toBool(false);
    descriptor.postprocess.maxDetections = postprocess.value("max_detections").toInt(300);

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
        result.errorMessage = "Failed to open model.json";
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        result.errorMessage = "model.json is not a valid JSON object";
        return result;
    }

    result.descriptor = fromJson(document.object());
    result.success = true;
    return result;
}

}
