#include "core/operators/builtin/MeanBlurOperator.h"

#include <opencv2/imgproc.hpp>

QString MeanBlurOperator::id() const
{
    return "builtin.mean_blur";
}

QString MeanBlurOperator::displayName() const
{
    return "Mean Blur";
}

StepSchema MeanBlurOperator::schema() const
{
    return {
        "builtin.mean_blur",
        "Mean Blur",
        {
            {"kernelWidth", "Kernel Width", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
            {"kernelHeight", "Kernel Height", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
        }
    };
}

QVariantMap MeanBlurOperator::parameterValues() const
{
    return {
        {"kernelWidth", m_kernelWidth},
        {"kernelHeight", m_kernelHeight},
    };
}

void MeanBlurOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("kernelWidth")) {
        m_kernelWidth = values.value("kernelWidth").toInt();
    }
    if (values.contains("kernelHeight")) {
        m_kernelHeight = values.value("kernelHeight").toInt();
    }
}

StepResult MeanBlurOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat blurred;
    cv::blur(frame.workingMat, blurred, cv::Size(std::max(1, m_kernelWidth), std::max(1, m_kernelHeight)));
    frame.workingMat = blurred;
    return {};
}
