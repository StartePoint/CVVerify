#include "core/detection/ModelImportService.h"

#include <QDir>
#include <QImage>
#include <QPainter>

#include "core/detection/YoloOnnxImportService.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace ModelImportService {

namespace {

QString sourceTypeLabel(ModelSourceType sourceType)
{
    switch (sourceType) {
    case ModelSourceType::YoloDetection:
        return "yolo_detection";
    case ModelSourceType::GenericDetection:
        return "generic_detection";
    case ModelSourceType::GenericSegmentation:
        return "generic_segmentation";
    case ModelSourceType::Classification:
        return "classification";
    case ModelSourceType::Ocr:
        return "ocr";
    case ModelSourceType::Keypoint:
        return "keypoint";
    case ModelSourceType::Custom:
        return "custom";
    }
    return "custom";
}

}

ModelImportResult importModel(const ModelImportRequest& request)
{
    ModelImportResult result;

    YoloImportRequest yoloRequest;
    yoloRequest.modelPath = request.modelPath;
    yoloRequest.labelsPath = request.labelsPath;
    yoloRequest.modelName = request.modelName;

    const YoloImportResult yoloResult = YoloOnnxImportService::buildImportDescriptor(yoloRequest);
    if (!yoloResult.success) {
        result.errorMessage = yoloResult.errorMessage;
        return result;
    }

    result.descriptor = yoloResult.descriptor;
    result.descriptor.frameworkFamily = sourceTypeLabel(request.sourceType);

    switch (request.sourceType) {
    case ModelSourceType::Classification:
        result.descriptor.taskType = "classification";
        result.descriptor.output.layoutType = "classification_logits";
        break;
    case ModelSourceType::GenericSegmentation:
        result.descriptor.taskType = "segmentation";
        result.descriptor.output.layoutType = "segmentation_mask";
        break;
    case ModelSourceType::GenericDetection:
        result.descriptor.taskType = "detection";
        result.descriptor.output.layoutType = "generic_detection";
        break;
    case ModelSourceType::Ocr:
        result.descriptor.taskType = "ocr";
        result.descriptor.output.layoutType = "ocr_sequence";
        break;
    case ModelSourceType::Keypoint:
        result.descriptor.taskType = "keypoint";
        result.descriptor.output.layoutType = "keypoint_heatmap";
        break;
    case ModelSourceType::Custom:
        result.descriptor.taskType = "custom";
        result.descriptor.output.layoutType = "custom_tensor";
        break;
    case ModelSourceType::YoloDetection:
    default:
        result.descriptor.taskType = "detection";
        result.descriptor.frameworkFamily = "yolo";
        result.descriptor.output.layoutType = "yolo_detect_auto";
        break;
    }

    DiagnosticsLog::instance().info(
        "model_import",
        QString("Imported %1 as %2").arg(result.descriptor.modelName, result.descriptor.taskType));

    result.success = true;
    return result;
}

bool writePreviewPlaceholder(const QString& packageDir, QString* errorMessage)
{
    QDir().mkpath(packageDir);
    const QString previewPath = QDir(packageDir).filePath("preview.png");

    QImage image(320, 200, QImage::Format_RGB32);
    image.fill(QColor(24, 33, 43));
    QPainter painter(&image);
    painter.setPen(QColor(221, 230, 238));
    painter.drawText(image.rect(), Qt::AlignCenter, "Model Preview");
    painter.end();

    if (!image.save(previewPath)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write preview image: %1").arg(previewPath);
        }
        return false;
    }

    return true;
}

}
