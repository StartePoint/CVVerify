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
