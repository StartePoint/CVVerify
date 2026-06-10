#include "core/operators/builtin/FiberGlowOperator.h"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureColorForFiberGlow(const cv::Mat& source)
{
    if (source.channels() == 3) {
        return source;
    }

    cv::Mat color;
    cv::cvtColor(source, color, cv::COLOR_GRAY2BGR);
    return color;
}

cv::Mat createLineKernel(int length, double angleDeg)
{
    const int kernelSize = std::max(3, (length % 2 == 0) ? length + 1 : length);
    cv::Mat kernel(kernelSize, kernelSize, CV_32F, cv::Scalar(0));
    const cv::Point center(kernelSize / 2, kernelSize / 2);
    const double radians = angleDeg * CV_PI / 180.0;
    const double dx = std::cos(radians) * static_cast<double>(kernelSize / 2);
    const double dy = std::sin(radians) * static_cast<double>(kernelSize / 2);
    const cv::Point start(
        static_cast<int>(std::round(center.x - dx)),
        static_cast<int>(std::round(center.y - dy))
    );
    const cv::Point end(
        static_cast<int>(std::round(center.x + dx)),
        static_cast<int>(std::round(center.y + dy))
    );
    cv::line(kernel, start, end, cv::Scalar(1.0F), 1, cv::LINE_AA);
    const double sum = cv::sum(kernel)[0];
    if (sum > 0.0) {
        kernel /= sum;
    }
    return kernel;
}
}

QString FiberGlowOperator::id() const
{
    return "builtin.fiber_glow";
}

QString FiberGlowOperator::displayName() const
{
    return "Fiber Glow";
}

StepSchema FiberGlowOperator::schema() const
{
    return {
        "builtin.fiber_glow",
        "Fiber Glow",
        {
            {"angleDeg", "Angle Deg", "效果设置", StepParameterType::Double, 25.0, 0.0, 360.0, 1.0, {}, {}, {}},
            {"streakLength", "Streak Length", "效果设置", StepParameterType::Integer, 15, 1, 200, 1, {}, {}, {}},
            {"intensity", "Intensity", "效果设置", StepParameterType::Double, 1.2, 0.0, 10.0, 0.1, {}, {}, {}},
            {"threshold", "Threshold", "效果设置", StepParameterType::Double, 180.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"blendAlpha", "Blend Alpha", "混合设置", StepParameterType::Double, 0.7, 0.0, 1.0, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap FiberGlowOperator::parameterValues() const
{
    return {
        {"angleDeg", m_angleDeg},
        {"streakLength", m_streakLength},
        {"intensity", m_intensity},
        {"threshold", m_threshold},
        {"blendAlpha", m_blendAlpha},
    };
}

void FiberGlowOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("angleDeg")) m_angleDeg = values.value("angleDeg").toDouble();
    if (values.contains("streakLength")) m_streakLength = values.value("streakLength").toInt();
    if (values.contains("intensity")) m_intensity = values.value("intensity").toDouble();
    if (values.contains("threshold")) m_threshold = values.value("threshold").toDouble();
    if (values.contains("blendAlpha")) m_blendAlpha = values.value("blendAlpha").toDouble();
}

StepResult FiberGlowOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    const cv::Mat color = ensureColorForFiberGlow(frame.workingMat);
    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    cv::Mat mask;
    cv::threshold(gray, mask, m_threshold, 255.0, cv::THRESH_BINARY);
    cv::Mat brightLayer;
    color.copyTo(brightLayer, mask);

    const cv::Mat kernel = createLineKernel(m_streakLength, m_angleDeg);
    cv::Mat glow;
    cv::filter2D(brightLayer, glow, -1, kernel);
    glow.convertTo(glow, -1, std::max(0.0, m_intensity), 0.0);

    cv::Mat output;
    cv::addWeighted(color, 1.0, glow, std::clamp(m_blendAlpha, 0.0, 1.0), 0.0, output);
    frame.workingMat = output;
    return {};
}
