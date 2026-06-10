#include "core/operators/builtin/CornerDetectOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureCornerInput(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString CornerDetectOperator::id() const
{
    return "builtin.corner_detect";
}

QString CornerDetectOperator::displayName() const
{
    return "Corner Detect";
}

StepSchema CornerDetectOperator::schema() const
{
    return {
        "builtin.corner_detect",
        "Corner Detect",
        {
            {"method", "Method", "检测设置", StepParameterType::Choice, "shi_tomasi", {}, {}, {}, {}, {}, {
                {"shi_tomasi", "Shi-Tomasi"},
                {"harris", "Harris"},
            }},
            {"maxCorners", "Max Corners", "检测设置", StepParameterType::Integer, 200, 1, 10000, 1, {}, {}, {}},
            {"qualityLevel", "Quality Level", "检测设置", StepParameterType::Double, 0.01, 0.001, 1.0, 0.001, {}, {}, {}},
            {"minDistance", "Min Distance", "检测设置", StepParameterType::Double, 10.0, 0.0, 1000.0, 1.0, {}, {}, {}},
            {"blockSize", "Block Size", "检测设置", StepParameterType::Integer, 3, 1, 31, 1, {}, {}, {}},
            {"harrisK", "Harris K", "Harris 设置", StepParameterType::Double, 0.04, 0.01, 0.2, 0.01, "method=harris", {}, {}},
        }
    };
}

QVariantMap CornerDetectOperator::parameterValues() const
{
    return {
        {"method", m_method},
        {"maxCorners", m_maxCorners},
        {"qualityLevel", m_qualityLevel},
        {"minDistance", m_minDistance},
        {"blockSize", m_blockSize},
        {"harrisK", m_harrisK},
    };
}

void CornerDetectOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("method")) m_method = values.value("method").toString();
    if (values.contains("maxCorners")) m_maxCorners = values.value("maxCorners").toInt();
    if (values.contains("qualityLevel")) m_qualityLevel = values.value("qualityLevel").toDouble();
    if (values.contains("minDistance")) m_minDistance = values.value("minDistance").toDouble();
    if (values.contains("blockSize")) m_blockSize = values.value("blockSize").toInt();
    if (values.contains("harrisK")) m_harrisK = values.value("harrisK").toDouble();
}

StepResult CornerDetectOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureCornerInput(frame.workingMat);
    std::vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(
        gray,
        corners,
        m_maxCorners,
        m_qualityLevel,
        m_minDistance,
        cv::noArray(),
        m_blockSize,
        m_method == "harris",
        m_harrisK
    );

    cv::Mat output;
    if (frame.workingMat.channels() == 1) {
        cv::cvtColor(frame.workingMat, output, cv::COLOR_GRAY2BGR);
    } else {
        output = frame.workingMat.clone();
    }

    for (const cv::Point2f& point : corners) {
        cv::circle(output, point, 3, cv::Scalar(0, 255, 0), cv::FILLED);
    }

    frame.metrics.insert("corner_count", static_cast<int>(corners.size()));
    frame.workingMat = output;
    return {};
}
