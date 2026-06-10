#include "core/operators/builtin/CannyOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int normalizeApertureSize(int value)
{
    int aperture = std::max(3, value);
    if (aperture % 2 == 0) {
        aperture += 1;
    }
    return std::min(aperture, 7);
}

cv::Mat ensureSingleChannelEdgeInput(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString CannyOperator::id() const
{
    return "builtin.canny";
}

QString CannyOperator::displayName() const
{
    return "Canny";
}

StepSchema CannyOperator::schema() const
{
    return {
        "builtin.canny",
        "Canny",
        {
            {"threshold1", "Threshold 1", "阈值设置", StepParameterType::Double, 50.0, 0.0, 500.0, 1.0, {}, {}, {}},
            {"threshold2", "Threshold 2", "阈值设置", StepParameterType::Double, 150.0, 0.0, 500.0, 1.0, {}, {}, {}},
            {"apertureSize", "Aperture Size", "梯度设置", StepParameterType::Integer, 3, 3, 7, 2, {}, "Must stay odd and within 3~7", {}},
            {"useL2Gradient", "Use L2 Gradient", "梯度设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap CannyOperator::parameterValues() const
{
    return {
        {"threshold1", m_threshold1},
        {"threshold2", m_threshold2},
        {"apertureSize", m_apertureSize},
        {"useL2Gradient", m_useL2Gradient},
    };
}

void CannyOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("threshold1")) {
        m_threshold1 = values.value("threshold1").toDouble();
    }
    if (values.contains("threshold2")) {
        m_threshold2 = values.value("threshold2").toDouble();
    }
    if (values.contains("apertureSize")) {
        m_apertureSize = values.value("apertureSize").toInt();
    }
    if (values.contains("useL2Gradient")) {
        m_useL2Gradient = values.value("useL2Gradient").toBool();
    }
}

StepResult CannyOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureSingleChannelEdgeInput(frame.workingMat);
    cv::Mat edges;
    cv::Canny(gray, edges, m_threshold1, m_threshold2, normalizeApertureSize(m_apertureSize), m_useL2Gradient);
    frame.workingMat = edges;
    return {};
}
