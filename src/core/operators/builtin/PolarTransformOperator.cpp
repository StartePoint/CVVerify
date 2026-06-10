#include "core/operators/builtin/PolarTransformOperator.h"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace {
double distanceToCorner(const cv::Point2f& center, int x, int y)
{
    const double dx = static_cast<double>(x) - center.x;
    const double dy = static_cast<double>(y) - center.y;
    return std::sqrt(dx * dx + dy * dy);
}

double autoMaxRadius(const cv::Mat& source, const cv::Point2f& center)
{
    return std::max({
        distanceToCorner(center, 0, 0),
        distanceToCorner(center, source.cols - 1, 0),
        distanceToCorner(center, 0, source.rows - 1),
        distanceToCorner(center, source.cols - 1, source.rows - 1),
    });
}

int interpolationFlagFromString(const QString& value)
{
    if (value == "nearest") return cv::INTER_NEAREST;
    if (value == "cubic") return cv::INTER_CUBIC;
    if (value == "area") return cv::INTER_AREA;
    if (value == "lanczos4") return cv::INTER_LANCZOS4;
    return cv::INTER_LINEAR;
}
}

QString PolarTransformOperator::id() const
{
    return "builtin.polar_transform";
}

QString PolarTransformOperator::displayName() const
{
    return "Polar Transform";
}

StepSchema PolarTransformOperator::schema() const
{
    return {
        "builtin.polar_transform",
        "Polar Transform",
        {
            {"direction", "Direction", "模式设置", StepParameterType::Choice, "cart_to_polar", {}, {}, {}, {}, {}, {
                {"cart_to_polar", "Cartesian To Polar"},
                {"polar_to_cart", "Polar To Cartesian"},
            }},
            {"polarMode", "Polar Mode", "模式设置", StepParameterType::Choice, "linear", {}, {}, {}, {}, {}, {
                {"linear", "Linear"},
                {"log", "Log"},
            }},
            {"centerMode", "Center Mode", "中心设置", StepParameterType::Choice, "image_center", {}, {}, {}, {}, {}, {
                {"image_center", "Image Center"},
                {"manual", "Manual"},
            }},
            {"centerX", "Center X", "中心设置", StepParameterType::Double, 0.0, 0.0, 10000.0, 1.0, "centerMode=manual", {}, {}},
            {"centerY", "Center Y", "中心设置", StepParameterType::Double, 0.0, 0.0, 10000.0, 1.0, "centerMode=manual", {}, {}},
            {"radiusMode", "Radius Mode", "中心设置", StepParameterType::Choice, "auto", {}, {}, {}, {}, {}, {
                {"auto", "Auto"},
                {"manual", "Manual"},
            }},
            {"maxRadius", "Max Radius", "中心设置", StepParameterType::Double, 0.0, 0.0, 10000.0, 1.0, "radiusMode=manual", {}, {}},
            {"outputWidth", "Output Width", "输出设置", StepParameterType::Integer, 0, 0, 8192, 1, {}, {}, {}},
            {"outputHeight", "Output Height", "输出设置", StepParameterType::Integer, 0, 0, 8192, 1, {}, {}, {}},
            {"interpolation", "Interpolation", "输出设置", StepParameterType::Choice, "linear", {}, {}, {}, {}, {}, {
                {"nearest", "Nearest"},
                {"linear", "Linear"},
                {"cubic", "Cubic"},
                {"area", "Area"},
                {"lanczos4", "Lanczos4"},
            }},
        }
    };
}

QVariantMap PolarTransformOperator::parameterValues() const
{
    return {
        {"direction", m_direction},
        {"polarMode", m_polarMode},
        {"centerMode", m_centerMode},
        {"centerX", m_centerX},
        {"centerY", m_centerY},
        {"radiusMode", m_radiusMode},
        {"maxRadius", m_maxRadius},
        {"outputWidth", m_outputWidth},
        {"outputHeight", m_outputHeight},
        {"interpolation", m_interpolation},
    };
}

void PolarTransformOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("direction")) m_direction = values.value("direction").toString();
    if (values.contains("polarMode")) m_polarMode = values.value("polarMode").toString();
    if (values.contains("centerMode")) m_centerMode = values.value("centerMode").toString();
    if (values.contains("centerX")) m_centerX = values.value("centerX").toDouble();
    if (values.contains("centerY")) m_centerY = values.value("centerY").toDouble();
    if (values.contains("radiusMode")) m_radiusMode = values.value("radiusMode").toString();
    if (values.contains("maxRadius")) m_maxRadius = values.value("maxRadius").toDouble();
    if (values.contains("outputWidth")) m_outputWidth = values.value("outputWidth").toInt();
    if (values.contains("outputHeight")) m_outputHeight = values.value("outputHeight").toInt();
    if (values.contains("interpolation")) m_interpolation = values.value("interpolation").toString();
}

StepResult PolarTransformOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    const cv::Point2f center = (m_centerMode == "manual")
        ? cv::Point2f(static_cast<float>(m_centerX), static_cast<float>(m_centerY))
        : cv::Point2f(static_cast<float>(frame.workingMat.cols) / 2.0F, static_cast<float>(frame.workingMat.rows) / 2.0F);

    const double maxRadius = (m_radiusMode == "manual" && m_maxRadius > 0.0)
        ? m_maxRadius
        : autoMaxRadius(frame.workingMat, center);
    if (maxRadius <= 0.0) {
        return {false, "Polar transform requires a positive radius"};
    }

    int outputWidth = m_outputWidth;
    int outputHeight = m_outputHeight;
    if (outputWidth <= 0 || outputHeight <= 0) {
        if (m_direction == "cart_to_polar") {
            outputWidth = std::max(1, static_cast<int>(std::round(maxRadius)));
            outputHeight = std::max(1, static_cast<int>(std::round(maxRadius * CV_PI)));
        } else {
            outputWidth = frame.workingMat.cols;
            outputHeight = frame.workingMat.rows;
        }
    }

    int flags = interpolationFlagFromString(m_interpolation);
    if (m_polarMode == "log") {
        flags |= cv::WARP_POLAR_LOG;
    }
    if (m_direction == "polar_to_cart") {
        flags |= cv::WARP_INVERSE_MAP;
    }

    cv::Mat transformed;
    cv::warpPolar(
        frame.workingMat,
        transformed,
        cv::Size(outputWidth, outputHeight),
        center,
        maxRadius,
        flags
    );
    frame.workingMat = transformed;
    return {};
}
