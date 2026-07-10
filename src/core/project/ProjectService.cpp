#include "core/project/ProjectService.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QJsonObject detectionModelToJson(const DetectionModelDescriptor& descriptor)
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

DetectionModelDescriptor detectionModelFromJson(const QJsonObject& root)
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

QJsonObject pipelineToJson(const PipelineDefinition& definition)
{
    QJsonObject root;
    root.insert("pipelineId", definition.pipelineId);
    root.insert("displayName", definition.displayName);

    QJsonArray stepsArray;
    for (const PipelineStepDefinition& step : definition.steps) {
        QJsonObject stepObject;
        stepObject.insert("stepId", step.stepId);
        stepObject.insert("displayName", step.displayName);
        stepObject.insert("parameters", QJsonObject::fromVariantMap(step.parameters));
        stepsArray.append(stepObject);
    }
    root.insert("steps", stepsArray);
    return root;
}

PipelineDefinition pipelineFromJson(const QJsonObject& root)
{
    PipelineDefinition definition;
    definition.pipelineId = root.value("pipelineId").toString();
    definition.displayName = root.value("displayName").toString();

    const QJsonArray stepsArray = root.value("steps").toArray();
    for (const QJsonValue& value : stepsArray) {
        const QJsonObject stepObject = value.toObject();
        PipelineStepDefinition step;
        step.stepId = stepObject.value("stepId").toString();
        step.displayName = stepObject.value("displayName").toString();
        step.parameters = stepObject.value("parameters").toObject().toVariantMap();
        definition.steps.append(step);
    }
    return definition;
}

}

namespace ProjectService {

bool saveToFile(const ProjectDefinition& project, const QString& filePath, QString* errorMessage)
{
    QJsonObject root;
    root.insert("projectId", project.projectId);
    root.insert("displayName", project.displayName);
    root.insert("version", project.version);
    root.insert("mediaSourcePath", project.mediaSourcePath);
    root.insert("mediaSourceKind", project.mediaSourceKind);
    root.insert("currentVideoFrameIndex", project.currentVideoFrameIndex);
    root.insert("pipeline", pipelineToJson(project.pipeline));
    root.insert("activeDetectionModel", detectionModelToJson(project.activeDetectionModel));
    root.insert("activeDetectionModelPath", project.activeDetectionModelPath);
    root.insert("activeDetectionModelPackageDir", project.activeDetectionModelPackageDir);
    root.insert("appSettings", QJsonObject{
        {"languageCode", project.appSettings.languageCode},
        {"defaultExportDirectory", project.appSettings.defaultExportDirectory},
        {"autoOpenExportDirectory", project.appSettings.autoOpenExportDirectory}
    });

    QJsonArray recentArray;
    for (const QString& path : project.recentMediaFiles) {
        recentArray.append(path);
    }
    root.insert("recentMediaFiles", recentArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open project file for writing: %1").arg(filePath);
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

ProjectLoadResult loadFromFile(const QString& filePath)
{
    ProjectLoadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = QString("Failed to open project file: %1").arg(filePath);
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.errorMessage = QString("Failed to parse project file: %1").arg(filePath);
        return result;
    }

    const QJsonObject root = document.object();
    result.project.projectId = root.value("projectId").toString();
    result.project.displayName = root.value("displayName").toString();
    result.project.version = root.value("version").toString("1.0");
    result.project.mediaSourcePath = root.value("mediaSourcePath").toString();
    result.project.mediaSourceKind = root.value("mediaSourceKind").toString();
    result.project.currentVideoFrameIndex = root.value("currentVideoFrameIndex").toInt(0);
    result.project.pipeline = pipelineFromJson(root.value("pipeline").toObject());
    result.project.activeDetectionModel = detectionModelFromJson(root.value("activeDetectionModel").toObject());
    result.project.activeDetectionModelPath = root.value("activeDetectionModelPath").toString();
    result.project.activeDetectionModelPackageDir = root.value("activeDetectionModelPackageDir").toString();

    const QJsonObject settings = root.value("appSettings").toObject();
    result.project.appSettings.languageCode = settings.value("languageCode").toString("en");
    result.project.appSettings.defaultExportDirectory = settings.value("defaultExportDirectory").toString();
    result.project.appSettings.autoOpenExportDirectory = settings.value("autoOpenExportDirectory").toBool(false);

    const QJsonArray recentArray = root.value("recentMediaFiles").toArray();
    for (const QJsonValue& value : recentArray) {
        result.project.recentMediaFiles.append(value.toString());
    }

    result.success = true;
    return result;
}

}
