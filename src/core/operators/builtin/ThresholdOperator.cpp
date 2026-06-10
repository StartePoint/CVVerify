#include "core/operators/builtin/ThresholdOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int thresholdTypeFromString(const QString& value)
{
    if (value == "binary_inv") {
        return cv::THRESH_BINARY_INV;
    }
    if (value == "trunc") {
        return cv::THRESH_TRUNC;
    }
    if (value == "tozero") {
        return cv::THRESH_TOZERO;
    }
    if (value == "tozero_inv") {
        return cv::THRESH_TOZERO_INV;
    }
    if (value == "otsu") {
        return cv::THRESH_BINARY | cv::THRESH_OTSU;
    }
    return cv::THRESH_BINARY;
}

cv::Mat ensureSingleChannel(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString ThresholdOperator::id() const
{
    return "builtin.threshold";
}

QString ThresholdOperator::displayName() const
{
    return "Threshold";
}

StepSchema ThresholdOperator::schema() const
{
    return {
        "builtin.threshold",
        "Threshold",
        {
            {"threshold", "Threshold", "阈值设置", StepParameterType::Double, 127.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"maxValue", "Max Value", "阈值设置", StepParameterType::Double, 255.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"type", "Type", "阈值设置", StepParameterType::Choice, "binary", {}, {}, {}, {}, {}, {
                {"binary", "Binary"},
                {"binary_inv", "Binary Inverted"},
                {"trunc", "Truncate"},
                {"tozero", "To Zero"},
                {"tozero_inv", "To Zero Inverted"},
                {"otsu", "Otsu"},
            }},
        }
    };
}

QVariantMap ThresholdOperator::parameterValues() const
{
    return {
        {"threshold", m_threshold},
        {"maxValue", m_maxValue},
        {"type", m_type},
    };
}

void ThresholdOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("threshold")) {
        m_threshold = values.value("threshold").toDouble();
    }
    if (values.contains("maxValue")) {
        m_maxValue = values.value("maxValue").toDouble();
    }
    if (values.contains("type")) {
        m_type = values.value("type").toString();
    }
}

StepResult ThresholdOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureSingleChannel(frame.workingMat);
    cv::Mat thresholded;
    cv::threshold(gray, thresholded, m_threshold, m_maxValue, thresholdTypeFromString(m_type));
    frame.workingMat = thresholded;
    return {};
}
