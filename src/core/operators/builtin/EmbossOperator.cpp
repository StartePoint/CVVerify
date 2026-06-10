#include "core/operators/builtin/EmbossOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat embossKernelFromDirection(const QString& direction)
{
    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
        -2.0F, -1.0F, 0.0F,
        -1.0F, 1.0F, 1.0F,
         0.0F, 1.0F, 2.0F);

    if (direction == "north_east") {
        cv::flip(kernel, kernel, 1);
    } else if (direction == "south_west") {
        cv::flip(kernel, kernel, 0);
    } else if (direction == "south_east") {
        cv::flip(kernel, kernel, -1);
    }
    return kernel;
}
}

QString EmbossOperator::id() const
{
    return "builtin.emboss";
}

QString EmbossOperator::displayName() const
{
    return "Emboss";
}

StepSchema EmbossOperator::schema() const
{
    return {
        "builtin.emboss",
        "Emboss",
        {
            {"direction", "Direction", "效果设置", StepParameterType::Choice, "north_west", {}, {}, {}, {}, {}, {
                {"north_west", "North West"},
                {"north_east", "North East"},
                {"south_west", "South West"},
                {"south_east", "South East"},
            }},
            {"strength", "Strength", "效果设置", StepParameterType::Double, 1.0, 0.0, 10.0, 0.1, {}, {}, {}},
            {"bias", "Bias", "效果设置", StepParameterType::Double, 128.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"convertToGray", "Convert To Gray", "效果设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap EmbossOperator::parameterValues() const
{
    return {
        {"direction", m_direction},
        {"strength", m_strength},
        {"bias", m_bias},
        {"convertToGray", m_convertToGray},
    };
}

void EmbossOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("direction")) m_direction = values.value("direction").toString();
    if (values.contains("strength")) m_strength = values.value("strength").toDouble();
    if (values.contains("bias")) m_bias = values.value("bias").toDouble();
    if (values.contains("convertToGray")) m_convertToGray = values.value("convertToGray").toBool();
}

StepResult EmbossOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat source = frame.workingMat;
    if (m_convertToGray && source.channels() != 1) {
        cv::cvtColor(frame.workingMat, source, cv::COLOR_BGR2GRAY);
    }

    cv::Mat filtered;
    cv::filter2D(
        source,
        filtered,
        CV_32F,
        embossKernelFromDirection(m_direction) * static_cast<float>(m_strength),
        cv::Point(-1, -1),
        m_bias
    );

    cv::Mat output;
    filtered.convertTo(output, source.type());
    frame.workingMat = output;
    return {};
}
