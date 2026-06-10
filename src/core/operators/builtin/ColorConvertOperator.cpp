#include "core/operators/builtin/ColorConvertOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
bool applyColorConversion(const cv::Mat& source, cv::Mat& destination, const QString& conversion)
{
    if (conversion == "bgr_to_gray") {
        cv::cvtColor(source, destination, cv::COLOR_BGR2GRAY);
        return true;
    }
    if (conversion == "gray_to_bgr") {
        if (source.channels() == 1) {
            cv::cvtColor(source, destination, cv::COLOR_GRAY2BGR);
            return true;
        }
        destination = source.clone();
        return true;
    }
    if (conversion == "bgr_to_rgb") {
        cv::cvtColor(source, destination, cv::COLOR_BGR2RGB);
        return true;
    }
    if (conversion == "rgb_to_bgr") {
        cv::cvtColor(source, destination, cv::COLOR_RGB2BGR);
        return true;
    }
    if (conversion == "bgr_to_hsv") {
        cv::cvtColor(source, destination, cv::COLOR_BGR2HSV);
        return true;
    }
    if (conversion == "hsv_to_bgr") {
        cv::cvtColor(source, destination, cv::COLOR_HSV2BGR);
        return true;
    }
    if (conversion == "bgr_to_lab") {
        cv::cvtColor(source, destination, cv::COLOR_BGR2Lab);
        return true;
    }
    if (conversion == "lab_to_bgr") {
        cv::cvtColor(source, destination, cv::COLOR_Lab2BGR);
        return true;
    }
    if (conversion == "bgr_to_ycrcb") {
        cv::cvtColor(source, destination, cv::COLOR_BGR2YCrCb);
        return true;
    }
    if (conversion == "ycrcb_to_bgr") {
        cv::cvtColor(source, destination, cv::COLOR_YCrCb2BGR);
        return true;
    }
    return false;
}
}

QString ColorConvertOperator::id() const
{
    return "builtin.color_convert";
}

QString ColorConvertOperator::displayName() const
{
    return "Color Convert";
}

StepSchema ColorConvertOperator::schema() const
{
    return {
        "builtin.color_convert",
        "Color Convert",
        {
            {"conversion", "Conversion", "颜色设置", StepParameterType::Choice, "bgr_to_gray", {}, {}, {}, {}, {}, {
                {"bgr_to_gray", "BGR to Gray"},
                {"gray_to_bgr", "Gray to BGR"},
                {"bgr_to_rgb", "BGR to RGB"},
                {"rgb_to_bgr", "RGB to BGR"},
                {"bgr_to_hsv", "BGR to HSV"},
                {"hsv_to_bgr", "HSV to BGR"},
                {"bgr_to_lab", "BGR to Lab"},
                {"lab_to_bgr", "Lab to BGR"},
                {"bgr_to_ycrcb", "BGR to YCrCb"},
                {"ycrcb_to_bgr", "YCrCb to BGR"},
            }},
        }
    };
}

QVariantMap ColorConvertOperator::parameterValues() const
{
    return {
        {"conversion", m_conversion},
    };
}

void ColorConvertOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("conversion")) {
        m_conversion = values.value("conversion").toString();
    }
}

StepResult ColorConvertOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat converted;
    if (!applyColorConversion(frame.workingMat, converted, m_conversion)) {
        return {false, "Unsupported color conversion"};
    }

    frame.workingMat = converted;
    return {};
}
