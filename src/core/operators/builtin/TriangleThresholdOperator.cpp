#include "core/operators/builtin/TriangleThresholdOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForTriangle(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int triangleThresholdTypeFromString(const QString& value)
{
    if (value == "binary_inv") {
        return cv::THRESH_BINARY_INV | cv::THRESH_TRIANGLE;
    }
    return cv::THRESH_BINARY | cv::THRESH_TRIANGLE;
}
}

QString TriangleThresholdOperator::id() const
{
    return "builtin.threshold_triangle";
}

QString TriangleThresholdOperator::displayName() const
{
    return "Triangle Threshold";
}

StepSchema TriangleThresholdOperator::schema() const
{
    return {
        "builtin.threshold_triangle",
        "Triangle Threshold",
        {
            {"maxValue", "Max Value", "阈值设置", StepParameterType::Double, 255.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"thresholdType", "Threshold Type", "阈值设置", StepParameterType::Choice, "binary", {}, {}, {}, {}, {}, {
                {"binary", "Binary"},
                {"binary_inv", "Binary Inverted"},
            }},
        }
    };
}

QVariantMap TriangleThresholdOperator::parameterValues() const
{
    return {
        {"maxValue", m_maxValue},
        {"thresholdType", m_thresholdType},
    };
}

void TriangleThresholdOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("maxValue")) {
        m_maxValue = values.value("maxValue").toDouble();
    }
    if (values.contains("thresholdType")) {
        m_thresholdType = values.value("thresholdType").toString();
    }
}

StepResult TriangleThresholdOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForTriangle(frame.workingMat);
    cv::Mat thresholded;
    cv::threshold(gray, thresholded, 0.0, m_maxValue, triangleThresholdTypeFromString(m_thresholdType));
    frame.workingMat = thresholded;
    return {};
}
