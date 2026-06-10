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
    } catch (const cv::Exception& ex) {
        result.errorMessage = QString::fromStdString(ex.what());
    }

    return result;
}

}
