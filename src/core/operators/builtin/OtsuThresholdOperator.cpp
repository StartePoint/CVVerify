#include "core/operators/builtin/OtsuThresholdOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForOtsu(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int otsuThresholdTypeFromString(const QString& value)
{
    if (value == "binary_inv") {
        return cv::THRESH_BINARY_INV | cv::THRESH_OTSU;
    }
    return cv::THRESH_BINARY | cv::THRESH_OTSU;
}
}

QString OtsuThresholdOperator::id() const
{
    return "builtin.threshold_otsu";
}

QString OtsuThresholdOperator::displayName() const
{
    return "Otsu Threshold";
}

StepSchema OtsuThresholdOperator::schema() const
{
    return {
        "builtin.threshold_otsu",
        "Otsu Threshold",
        {
            {"maxValue", "Max Value", "阈值设置", StepParameterType::Double, 255.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"thresholdType", "Threshold Type", "阈值设置", StepParameterType::Choice, "binary", {}, {}, {}, {}, {}, {
                {"binary", "Binary"},
                {"binary_inv", "Binary Inverted"},
            }},
        }
    };
}

QVariantMap OtsuThresholdOperator::parameterValues() const
{
    return {
        {"maxValue", m_maxValue},
        {"thresholdType", m_thresholdType},
    };
}

void OtsuThresholdOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("maxValue")) {
        m_maxValue = values.value("maxValue").toDouble();
    }
    if (values.contains("thresholdType")) {
        m_thresholdType = values.value("thresholdType").toString();
    }
}

StepResult OtsuThresholdOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForOtsu(frame.workingMat);
    cv::Mat thresholded;
    cv::threshold(gray, thresholded, 0.0, m_maxValue, otsuThresholdTypeFromString(m_thresholdType));
    frame.workingMat = thresholded;
    return {};
}
