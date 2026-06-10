#include "core/operators/builtin/GaussianBlurOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int normalizeOddKernel(int value)
{
    int kernel = std::max(1, value);
    if (kernel % 2 == 0) {
        kernel += 1;
    }
    return kernel;
}
}

QString GaussianBlurOperator::id() const
{
    return "builtin.gaussian_blur";
}

QString GaussianBlurOperator::displayName() const
{
    return "Gaussian Blur";
}

StepSchema GaussianBlurOperator::schema() const
{
    return {
        "builtin.gaussian_blur",
        "Gaussian Blur",
        {
            {"kernelWidth", "Kernel Width", "核设置", StepParameterType::Integer, 5, 1, 99, 1, {}, {}, {}},
            {"kernelHeight", "Kernel Height", "核设置", StepParameterType::Integer, 5, 1, 99, 1, {}, {}, {}},
            {"sigmaX", "Sigma X", "高斯设置", StepParameterType::Double, 1.0, 0.0, 50.0, 0.1, {}, {}, {}},
            {"sigmaY", "Sigma Y", "高斯设置", StepParameterType::Double, 0.0, 0.0, 50.0, 0.1, {}, "0 means follow Sigma X", {}},
        }
    };
}

QVariantMap GaussianBlurOperator::parameterValues() const
{
    return {
        {"kernelWidth", m_kernelWidth},
        {"kernelHeight", m_kernelHeight},
        {"sigmaX", m_sigmaX},
        {"sigmaY", m_sigmaY},
    };
}

void GaussianBlurOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("kernelWidth")) {
        m_kernelWidth = values.value("kernelWidth").toInt();
    }
    if (values.contains("kernelHeight")) {
        m_kernelHeight = values.value("kernelHeight").toInt();
    }
    if (values.contains("sigmaX")) {
        m_sigmaX = values.value("sigmaX").toDouble();
    }
    if (values.contains("sigmaY")) {
        m_sigmaY = values.value("sigmaY").toDouble();
    }
}

StepResult GaussianBlurOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat blurred;
    const int kernelWidth = normalizeOddKernel(m_kernelWidth);
    const int kernelHeight = normalizeOddKernel(m_kernelHeight);
    cv::GaussianBlur(frame.workingMat, blurred, cv::Size(kernelWidth, kernelHeight), m_sigmaX, m_sigmaY);
    frame.workingMat = blurred;
    return {};
}
