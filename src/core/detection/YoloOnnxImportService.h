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
