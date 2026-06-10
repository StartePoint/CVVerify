#include "core/operators/builtin/AdaptiveThresholdOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForAdaptive(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int adaptiveMethodFromString(const QString& value)
{
    if (value == "mean") {
        return cv::ADAPTIVE_THRESH_MEAN_C;
    }
    return cv::ADAPTIVE_THRESH_GAUSSIAN_C;
}

int adaptiveThresholdTypeFromString(const QString& value)
{
    if (value == "binary_inv") {
        return cv::THRESH_BINARY_INV;
    }
    return cv::THRESH_BINARY;
}

int normalizeAdaptiveBlockSize(int value)
{
    int blockSize = std::max(3, value);
    if (blockSize % 2 == 0) {
        blockSize += 1;
    }
    return blockSize;
}
}

QString AdaptiveThresholdOperator::id() const
{
    return "builtin.threshold_adaptive";
}

QString AdaptiveThresholdOperator::displayName() const
{
    return "Adaptive Threshold";
}

StepSchema AdaptiveThresholdOperator::schema() const
{
    return {
        "builtin.threshold_adaptive",
        "Adaptive Threshold",
        {
            {"maxValue", "Max Value", "阈值设置", StepParameterType::Double, 255.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"adaptiveMethod", "Adaptive Method", "阈值设置", StepParameterType::Choice, "gaussian", {}, {}, {}, {}, {}, {
                {"mean", "Mean"},
                {"gaussian", "Gaussian"},
            }},
            {"thresholdType", "Threshold Type", "阈值设置", StepParameterType::Choice, "binary", {}, {}, {}, {}, {}, {
                {"binary", "Binary"},
                {"binary_inv", "Binary Inverted"},
            }},
            {"blockSize", "Block Size", "阈值设置", StepParameterType::Integer, 11, 3, 99, 2, {}, "Must stay odd", {}},
            {"cValue", "C Value", "阈值设置", StepParameterType::Double, 2.0, -50.0, 50.0, 0.5, {}, {}, {}},
        }
    };
}

QVariantMap AdaptiveThresholdOperator::parameterValues() const
{
    return {
        {"maxValue", m_maxValue},
        {"adaptiveMethod", m_adaptiveMethod},
        {"thresholdType", m_thresholdType},
        {"blockSize", m_blockSize},
        {"cValue", m_cValue},
    };
}

void AdaptiveThresholdOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("maxValue")) {
        m_maxValue = values.value("maxValue").toDouble();
    }
    if (values.contains("adaptiveMethod")) {
        m_adaptiveMethod = values.value("adaptiveMethod").toString();
    }
    if (values.contains("thresholdType")) {
        m_thresholdType = values.value("thresholdType").toString();
    }
    if (values.contains("blockSize")) {
        m_blockSize = values.value("blockSize").toInt();
    }
    if (values.contains("cValue")) {
        m_cValue = values.value("cValue").toDouble();
    }
}

StepResult AdaptiveThresholdOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForAdaptive(frame.workingMat);
    cv::Mat thresholded;
    cv::adaptiveThreshold(
        gray,
        thresholded,
        m_maxValue,
        adaptiveMethodFromString(m_adaptiveMethod),
        adaptiveThresholdTypeFromString(m_thresholdType),
        normalizeAdaptiveBlockSize(m_blockSize),
        m_cValue
    );
    frame.workingMat = thresholded;
    return {};
}
