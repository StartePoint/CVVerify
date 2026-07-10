#include "core/detection/ModelTemplateLoader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/detection/ModelImportService.h"
#include "infra/platform/PlatformPaths.h"

namespace ModelTemplateLoader {

namespace {

QString readString(const QJsonObject& object, const QString& camelKey, const QString& snakeKey, const QString& fallback = QString())
{
    if (object.contains(camelKey)) {
        return object.value(camelKey).toString();
    }
    if (object.contains(snakeKey)) {
        return object.value(snakeKey).toString();
    }
    return fallback;
}

int readInt(const QJsonObject& object, const QString& camelKey, const QString& snakeKey, int fallback)
{
    if (object.contains(camelKey)) {
        return object.value(camelKey).toInt(fallback);
    }
    if (object.contains(snakeKey)) {
        return object.value(snakeKey).toInt(fallback);
    }
    return fallback;
}

double readDouble(const QJsonObject& object, const QString& camelKey, const QString& snakeKey, double fallback)
{
    if (object.contains(camelKey)) {
        return object.value(camelKey).toDouble(fallback);
    }
    if (object.contains(snakeKey)) {
        return object.value(snakeKey).toDouble(fallback);
    }
    return fallback;
}

bool readBool(const QJsonObject& object, const QString& camelKey, const QString& snakeKey, bool fallback)
{
    if (object.contains(camelKey)) {
        return object.value(camelKey).toBool(fallback);
    }
    if (object.contains(snakeKey)) {
        return object.value(snakeKey).toBool(fallback);
    }
    return fallback;
}

DetectionModelDescriptor descriptorFromJson(const QJsonObject& root)
{
    DetectionModelDescriptor descriptor;
    descriptor.modelName = readString(root, "modelName", "model_name");
    descriptor.taskType = readString(root, "taskType", "task_type", "detection");
    descriptor.frameworkFamily = readString(root, "frameworkFamily", "framework_family", "yolo");
    descriptor.modelFormat = readString(root, "modelFormat", "model_format", "onnx");
    descriptor.labelsPath = readString(root, "labelsPath", "labels_path", "labels.txt");

    const QJsonObject input = root.value("input").toObject();
    descriptor.input.width = readInt(input, "width", "width", 640);
    descriptor.input.height = readInt(input, "height", "height", 640);
    descriptor.input.channels = readInt(input, "channels", "channels", 3);
    descriptor.input.colorOrder = readString(input, "colorOrder", "color_order", "RGB");
    descriptor.input.letterbox = readBool(input, "letterbox", "letterbox", true);
    descriptor.input.keepRatio = readBool(input, "keepRatio", "keep_ratio", true);
    descriptor.input.scale = static_cast<float>(readDouble(input, "scale", "scale", 0.0039215686));

    const QJsonObject postprocess = root.value("postprocess").toObject();
    descriptor.postprocess.confidenceThreshold = static_cast<float>(readDouble(
        postprocess, "confidenceThreshold", "confidence_threshold", 0.25));
    descriptor.postprocess.nmsIouThreshold = static_cast<float>(readDouble(
        postprocess, "nmsIouThreshold", "nms_iou_threshold", 0.45));
    descriptor.postprocess.multiLabel = readBool(postprocess, "multiLabel", "multi_label", false);
    descriptor.postprocess.maxDetections = readInt(postprocess, "maxDetections", "max_detections", 300);

    const QJsonObject output = root.value("output").toObject();
    descriptor.output.layoutType = readString(output, "layoutType", "layout_type", "yolo_detect_auto");
    return descriptor;
}

QString templateDirectory()
{
    return PlatformPaths::resolveResourcePath("config/model_templates");
}

}

QStringList listTemplates()
{
    const QDir directory(templateDirectory());
    const QFileInfoList files = directory.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name);
    QStringList names;
    for (const QFileInfo& fileInfo : files) {
        names.append(fileInfo.completeBaseName());
    }
    return names;
}

bool loadTemplate(const QString& templateName, DetectionModelDescriptor* descriptor, QString* errorMessage)
{
    if (!descriptor) {
        if (errorMessage) {
            *errorMessage = "Template descriptor output is null";
        }
        return false;
    }

    const QString trimmedName = templateName.trimmed();
    if (trimmedName.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Template name is empty";
        }
        return false;
    }

    const QString filePath = QDir(templateDirectory()).filePath(trimmedName + ".json");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open template: %1").arg(filePath);
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid template JSON: %1").arg(filePath);
        }
        return false;
    }

    *descriptor = descriptorFromJson(document.object());
    return true;
}

void applyTemplateDefaults(DetectionModelDescriptor& target, const DetectionModelDescriptor& templateDescriptor)
{
    const QString modelName = target.modelName;
    target = templateDescriptor;
    if (!modelName.isEmpty()) {
        target.modelName = modelName;
    }
}

ModelSourceType sourceTypeFromTaskType(const QString& taskType)
{
    const QString normalized = taskType.trimmed().toLower();
    if (normalized == "classification") {
        return ModelSourceType::Classification;
    }
    if (normalized == "segmentation" || normalized == "semantic_segmentation" || normalized == "instance_segmentation") {
        return ModelSourceType::GenericSegmentation;
    }
    if (normalized == "ocr") {
        return ModelSourceType::Ocr;
    }
    if (normalized == "keypoint" || normalized == "keypoints") {
        return ModelSourceType::Keypoint;
    }
    if (normalized == "custom") {
        return ModelSourceType::Custom;
    }
    if (normalized == "detection" || normalized == "object_detection") {
        return ModelSourceType::GenericDetection;
    }
    return ModelSourceType::YoloDetection;
}

}
