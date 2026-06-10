#include "core/operators/builtin/MedianBlurOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int normalizeMedianKernel(int value)
{
    int kernel = std::max(3, value);
    if (kernel % 2 == 0) {
        kernel += 1;
    }
    return kernel;
}
}

QString MedianBlurOperator::id() const
{
    return "builtin.median_blur";
}

QString MedianBlurOperator::displayName() const
{
    return "Median Blur";
}

StepSchema MedianBlurOperator::schema() const
{
    return {
        "builtin.median_blur",
        "Median Blur",
        {
            {"kernelSize", "Kernel Size", "核设置", StepParameterType::Integer, 5, 3, 31, 2, {}, "Kernel size must stay odd", {}},
        }
    };
}

QVariantMap MedianBlurOperator::parameterValues() const
{
    return {
        {"kernelSize", m_kernelSize},
    };
}

void MedianBlurOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("kernelSize")) {
        m_kernelSize = values.value("kernelSize").toInt();
    }
}

StepResult MedianBlurOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat filtered;
    cv::medianBlur(frame.workingMat, filtered, normalizeMedianKernel(m_kernelSize));
    frame.workingMat = filtered;
    return {};
}
