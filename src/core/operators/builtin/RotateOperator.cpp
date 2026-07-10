#include "core/operators/builtin/RotateOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int borderTypeFromString(const QString& value)
{
    if (value == "replicate") return cv::BORDER_REPLICATE;
    if (value == "reflect") return cv::BORDER_REFLECT;
    if (value == "reflect101") return cv::BORDER_REFLECT_101;
    return cv::BORDER_CONSTANT;
}
}

QString RotateOperator::id() const
{
    return "builtin.rotate";
}

QString RotateOperator::displayName() const
{
    return "Rotate";
}

StepSchema RotateOperator::schema() const
{
    return {
        "builtin.rotate",
        "Rotate",
        {
            {"angle", "Angle", "旋转设置", StepParameterType::Double, 0.0, -360.0, 360.0, 0.1, {}, "Rotation angle in degrees", {}},
            {"scale", "Scale", "旋转设置", StepParameterType::Double, 1.0, 0.01, 10.0, 0.01, {}, "Optional uniform scale factor", {}},
            {"borderMode", "Border Mode", "输出设置", StepParameterType::Choice, "constant", {}, {}, {}, {}, {}, {
                {"constant", "Constant"},
                {"replicate", "Replicate"},
                {"reflect", "Reflect"},
                {"reflect101", "Reflect 101"},
            }},
            {"borderValue", "Border Value", "输出设置", StepParameterType::Integer, 0, 0, 255, 1, "borderMode=constant", {}, {}},
        }
    };
}

QVariantMap RotateOperator::parameterValues() const
{
    return {
        {"angle", m_angle},
        {"scale", m_scale},
        {"borderMode", m_borderMode},
        {"borderValue", m_borderValue},
    };
}

void RotateOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("angle")) {
        m_angle = values.value("angle").toDouble();
    }
    if (values.contains("scale")) {
        m_scale = values.value("scale").toDouble();
    }
    if (values.contains("borderMode")) {
        m_borderMode = values.value("borderMode").toString();
    }
    if (values.contains("borderValue")) {
        m_borderValue = values.value("borderValue").toInt();
    }
}

StepResult RotateOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (m_scale <= 0.0) {
        return {false, "Scale must be positive"};
    }

    const cv::Point2f center(
        static_cast<float>(frame.workingMat.cols) * 0.5f,
        static_cast<float>(frame.workingMat.rows) * 0.5f
    );
    const cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, m_angle, m_scale);

    cv::Mat rotated;
    const int borderType = borderTypeFromString(m_borderMode);
    const cv::Scalar borderColor(m_borderValue, m_borderValue, m_borderValue, m_borderValue);
    cv::warpAffine(
        frame.workingMat,
        rotated,
        rotationMatrix,
        frame.workingMat.size(),
        cv::INTER_LINEAR,
        borderType,
        borderColor
    );

    frame.workingMat = rotated;
    return {};
}
