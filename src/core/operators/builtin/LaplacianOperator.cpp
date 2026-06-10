#include "core/operators/builtin/LaplacianOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForLaplacian(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int normalizeLaplacianKernelSize(int value)
{
    if (value <= 1) {
        return 1;
    }
    int kernelSize = std::max(3, value);
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }
    return std::min(kernelSize, 7);
}
}

QString LaplacianOperator::id() const
{
    return "builtin.edge_laplacian";
}

QString LaplacianOperator::displayName() const
{
    return "Laplacian";
}

StepSchema LaplacianOperator::schema() const
{
    return {
        "builtin.edge_laplacian",
        "Laplacian",
        {
            {"kernelSize", "Kernel Size", "梯度设置", StepParameterType::Integer, 3, 1, 7, 2, {}, "1 or odd values up to 7", {}},
            {"scale", "Scale", "梯度设置", StepParameterType::Double, 1.0, 0.0, 20.0, 0.1, {}, {}, {}},
            {"delta", "Delta", "梯度设置", StepParameterType::Double, 0.0, -255.0, 255.0, 1.0, {}, {}, {}},
            {"convertAbs", "Convert Abs", "输出设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap LaplacianOperator::parameterValues() const
{
    return {
        {"kernelSize", m_kernelSize},
        {"scale", m_scale},
        {"delta", m_delta},
        {"convertAbs", m_convertAbs},
    };
}

void LaplacianOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("kernelSize")) {
        m_kernelSize = values.value("kernelSize").toInt();
    }
    if (values.contains("scale")) {
        m_scale = values.value("scale").toDouble();
    }
    if (values.contains("delta")) {
        m_delta = values.value("delta").toDouble();
    }
    if (values.contains("convertAbs")) {
        m_convertAbs = values.value("convertAbs").toBool();
    }
}

StepResult LaplacianOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForLaplacian(frame.workingMat);
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_16S, normalizeLaplacianKernelSize(m_kernelSize), m_scale, m_delta);

    if (m_convertAbs) {
        cv::Mat absOutput;
        cv::convertScaleAbs(laplacian, absOutput);
        frame.workingMat = absOutput;
    } else {
        laplacian.convertTo(frame.workingMat, gray.type());
    }
    return {};
}
