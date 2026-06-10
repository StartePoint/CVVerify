#include "core/operators/builtin/HoughCirclesOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureHoughCircleInput(const cv::Mat& source)
{
    cv::Mat gray;
    if (source.channels() == 1) {
        gray = source;
    } else {
        cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    }

    cv::medianBlur(gray, gray, 5);
    return gray;
}
}

QString HoughCirclesOperator::id() const
{
    return "builtin.hough_circles";
}

QString HoughCirclesOperator::displayName() const
{
    return "Hough Circles";
}

StepSchema HoughCirclesOperator::schema() const
{
    return {
        "builtin.hough_circles",
        "Hough Circles",
        {
            {"dp", "Dp", "检测设置", StepParameterType::Double, 1.0, 1.0, 10.0, 0.1, {}, {}, {}},
            {"minDist", "Min Dist", "检测设置", StepParameterType::Double, 20.0, 0.0, 100000.0, 1.0, {}, {}, {}},
            {"param1", "Param1", "检测设置", StepParameterType::Double, 100.0, 0.0, 500.0, 1.0, {}, {}, {}},
            {"param2", "Param2", "检测设置", StepParameterType::Double, 30.0, 0.0, 500.0, 1.0, {}, {}, {}},
            {"minRadius", "Min Radius", "检测设置", StepParameterType::Integer, 0, 0, 100000, 1, {}, {}, {}},
            {"maxRadius", "Max Radius", "检测设置", StepParameterType::Integer, 0, 0, 100000, 1, {}, {}, {}},
        }
    };
}

QVariantMap HoughCirclesOperator::parameterValues() const
{
    return {
        {"dp", m_dp},
        {"minDist", m_minDist},
        {"param1", m_param1},
        {"param2", m_param2},
        {"minRadius", m_minRadius},
        {"maxRadius", m_maxRadius},
    };
}

void HoughCirclesOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("dp")) m_dp = values.value("dp").toDouble();
    if (values.contains("minDist")) m_minDist = values.value("minDist").toDouble();
    if (values.contains("param1")) m_param1 = values.value("param1").toDouble();
    if (values.contains("param2")) m_param2 = values.value("param2").toDouble();
    if (values.contains("minRadius")) m_minRadius = values.value("minRadius").toInt();
    if (values.contains("maxRadius")) m_maxRadius = values.value("maxRadius").toInt();
}

StepResult HoughCirclesOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureHoughCircleInput(frame.workingMat);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, m_dp, m_minDist, m_param1, m_param2, m_minRadius, m_maxRadius);

    cv::Mat output;
    if (frame.workingMat.channels() == 1) {
        cv::cvtColor(frame.workingMat, output, cv::COLOR_GRAY2BGR);
    } else {
        output = frame.workingMat.clone();
    }

    for (const cv::Vec3f& circle : circles) {
        cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        int radius = cvRound(circle[2]);
        cv::circle(output, center, radius, cv::Scalar(0, 255, 0), 2);
        cv::circle(output, center, 2, cv::Scalar(0, 0, 255), 3);
    }

    frame.metrics.insert("hough_circle_count", static_cast<int>(circles.size()));
    frame.workingMat = output;
    return {};
}
