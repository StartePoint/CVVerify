#include "core/detection/YoloOnnxImportService.h"

#include <QFileInfo>

#include <opencv2/dnn.hpp>

#include "core/detection/LabelProvider.h"
#include "infra/opencv/OpenCvImageIO.h"

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

    try {
        cv::dnn::Net net = cv::dnn::readNet(OpenCvImageIO::toOpenCvFilePath(request.modelPath));
        if (!net.empty()) {
            result.descriptor.input.width = 640;
            result.descriptor.input.height = 640;
            result.descriptor.input.channels = 3;
            result.descriptor.input.colorOrder = "RGB";
            result.descriptor.input.letterbox = true;
            result.descriptor.input.keepRatio = true;
            result.descriptor.input.scale = 1.0f / 255.0f;
        }
    } catch (const cv::Exception&) {
        result.errorMessage = QString("Failed to read ONNX model: %1").arg(request.modelPath);
        return result;
    }

    result.success = true;
    return result;
}

}
