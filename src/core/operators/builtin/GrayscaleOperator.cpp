#include "core/operators/builtin/GrayscaleOperator.h"

#include <opencv2/imgproc.hpp>

QString GrayscaleOperator::id() const
{
    return "builtin.grayscale";
}

QString GrayscaleOperator::displayName() const
{
    return "Grayscale";
}

StepSchema GrayscaleOperator::schema() const
{
    return {"builtin.grayscale", "Grayscale", {}};
}

QVariantMap GrayscaleOperator::parameterValues() const
{
    return {};
}

void GrayscaleOperator::setParameterValues(const QVariantMap&)
{
}

StepResult GrayscaleOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (frame.workingMat.channels() == 1) {
        return {};
    }

    cv::Mat gray;
    cv::cvtColor(frame.workingMat, gray, cv::COLOR_BGR2GRAY);
    frame.workingMat = gray;
    return {};
}
