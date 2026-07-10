#pragma once

#include <QString>

#include "core/detection/DetectionModelDescriptor.h"
#include "core/detection/YoloOnnxImportService.h"

enum class ModelSourceType
{
    YoloDetection,
    GenericDetection,
    GenericSegmentation,
    Classification,
    Ocr,
    Keypoint,
    Custom
};

struct ModelImportRequest
{
    QString modelPath;
    QString labelsPath;
    QString modelName;
    ModelSourceType sourceType = ModelSourceType::YoloDetection;
};

struct ModelImportResult
{
    bool success = false;
    QString errorMessage;
    DetectionModelDescriptor descriptor;
};

namespace ModelImportService {

ModelImportResult importModel(const ModelImportRequest& request);
bool writePreviewPlaceholder(const QString& packageDir, QString* errorMessage = nullptr);

}
