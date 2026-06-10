#include "core/operators/builtin/SobelOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForSobel(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int normalizeSobelKernelSize(int value)
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

QString SobelOperator::id() const
{
    return "builtin.edge_sobel";
}

QString SobelOperator::displayName() const
{
    return "Sobel";
}

StepSchema SobelOperator::schema() const
{
    return {
        "builtin.edge_sobel",
        "Sobel",
        {
            {"outputMode", "Output Mode", "梯度设置", StepParameterType::Choice, "magnitude", {}, {}, {}, {}, {}, {
                {"x", "X"},
                {"y", "Y"},
                {"magnitude", "Magnitude"},
                {"abs_sum", "Abs Sum"},
            }},
            {"dx", "Dx", "梯度设置", StepParameterType::Integer, 1, 0, 3, 1, {}, {}, {}},
            {"dy", "Dy", "梯度设置", StepParameterType::Integer, 1, 0, 3, 1, {}, {}, {}},
            {"kernelSize", "Kernel Size", "梯度设置", StepParameterType::Integer, 3, 1, 7, 2, {}, "1 or odd values up to 7", {}},
            {"scale", "Scale", "梯度设置", StepParameterType::Double, 1.0, 0.0, 20.0, 0.1, {}, {}, {}},
            {"delta", "Delta", "梯度设置", StepParameterType::Double, 0.0, -255.0, 255.0, 1.0, {}, {}, {}},
        }
    };
}

QVariantMap SobelOperator::parameterValues() const
{
    return {
        {"outputMode", m_outputMode},
        {"dx", m_dx},
        {"dy", m_dy},
        {"kernelSize", m_kernelSize},
        {"scale", m_scale},
        {"delta", m_delta},
    };
}

void SobelOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("outputMode")) {
        m_outputMode = values.value("outputMode").toString();
    }
    if (values.contains("dx")) {
        m_dx = values.value("dx").toInt();
    }
    if (values.contains("dy")) {
        m_dy = values.value("dy").toInt();
    }
    if (values.contains("kernelSize")) {
        m_kernelSize = values.value("kernelSize").toInt();
    }
    if (values.contains("scale")) {
        m_scale = values.value("scale").toDouble();
    }
    if (values.contains("delta")) {
        m_delta = values.value("delta").toDouble();
    }
}

StepResult SobelOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForSobel(frame.workingMat);
    cv::Mat gradX;
    cv::Mat gradY;
    const int kernelSize = normalizeSobelKernelSize(m_kernelSize);

    cv::Sobel(gray, gradX, CV_16S, m_dx, 0, kernelSize, m_scale, m_delta);
    cv::Sobel(gray, gradY, CV_16S, 0, m_dy, kernelSize, m_scale, m_delta);

    cv::Mat absX;
    cv::Mat absY;
    cv::convertScaleAbs(gradX, absX);
    cv::convertScaleAbs(gradY, absY);

    if (m_outputMode == "x") {
        frame.workingMat = absX;
        return {};
    }
    if (m_outputMode == "y") {
        frame.workingMat = absY;
        return {};
    }

    cv::Mat result;
    if (m_outputMode == "abs_sum") {
        cv::addWeighted(absX, 0.5, absY, 0.5, 0.0, result);
    } else {
        cv::Mat magX;
        cv::Mat magY;
        absX.convertTo(magX, CV_32F);
        absY.convertTo(magY, CV_32F);
        cv::Mat magnitude;
        cv::magnitude(magX, magY, magnitude);
        cv::convertScaleAbs(magnitude, result);
    }

    frame.workingMat = result;
    return {};
}
