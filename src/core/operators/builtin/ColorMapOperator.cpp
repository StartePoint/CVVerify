#include "core/operators/builtin/ColorMapOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForColorMap(const cv::Mat& source, bool convertToGray)
{
    if (source.channels() == 1 || !convertToGray) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int colorMapFromString(const QString& value)
{
    if (value == "autumn") return cv::COLORMAP_AUTUMN;
    if (value == "bone") return cv::COLORMAP_BONE;
    if (value == "jet") return cv::COLORMAP_JET;
    if (value == "winter") return cv::COLORMAP_WINTER;
    if (value == "rainbow") return cv::COLORMAP_RAINBOW;
    if (value == "ocean") return cv::COLORMAP_OCEAN;
    if (value == "summer") return cv::COLORMAP_SUMMER;
    if (value == "spring") return cv::COLORMAP_SPRING;
    if (value == "cool") return cv::COLORMAP_COOL;
    if (value == "hsv") return cv::COLORMAP_HSV;
    if (value == "pink") return cv::COLORMAP_PINK;
    if (value == "hot") return cv::COLORMAP_HOT;
    if (value == "parula") return cv::COLORMAP_PARULA;
    if (value == "magma") return cv::COLORMAP_MAGMA;
    if (value == "inferno") return cv::COLORMAP_INFERNO;
    if (value == "plasma") return cv::COLORMAP_PLASMA;
    if (value == "viridis") return cv::COLORMAP_VIRIDIS;
    if (value == "cividis") return cv::COLORMAP_CIVIDIS;
    if (value == "deepgreen") return cv::COLORMAP_DEEPGREEN;
    return cv::COLORMAP_TURBO;
}
}

QString ColorMapOperator::id() const
{
    return "builtin.color_map";
}

QString ColorMapOperator::displayName() const
{
    return "Color Map";
}

StepSchema ColorMapOperator::schema() const
{
    return {
        "builtin.color_map",
        "Color Map",
        {
            {"colorMap", "Color Map", "映射设置", StepParameterType::Choice, "turbo", {}, {}, {}, {}, {}, {
                {"autumn", "Autumn"},
                {"bone", "Bone"},
                {"jet", "Jet"},
                {"winter", "Winter"},
                {"rainbow", "Rainbow"},
                {"ocean", "Ocean"},
                {"summer", "Summer"},
                {"spring", "Spring"},
                {"cool", "Cool"},
                {"hsv", "HSV"},
                {"pink", "Pink"},
                {"hot", "Hot"},
                {"parula", "Parula"},
                {"magma", "Magma"},
                {"inferno", "Inferno"},
                {"plasma", "Plasma"},
                {"viridis", "Viridis"},
                {"cividis", "Cividis"},
                {"turbo", "Turbo"},
                {"deepgreen", "Deep Green"},
            }},
            {"convertToGray", "Convert To Gray", "映射设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
            {"invertGray", "Invert Gray", "映射设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap ColorMapOperator::parameterValues() const
{
    return {
        {"colorMap", m_colorMap},
        {"convertToGray", m_convertToGray},
        {"invertGray", m_invertGray},
    };
}

void ColorMapOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("colorMap")) {
        m_colorMap = values.value("colorMap").toString();
    }
    if (values.contains("convertToGray")) {
        m_convertToGray = values.value("convertToGray").toBool();
    }
    if (values.contains("invertGray")) {
        m_invertGray = values.value("invertGray").toBool();
    }
}

StepResult ColorMapOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForColorMap(frame.workingMat, m_convertToGray);
    if (gray.channels() != 1) {
        return {false, "Color map requires single-channel input or conversion to gray"};
    }

    if (m_invertGray) {
        cv::bitwise_not(gray, gray);
    }

    cv::Mat colored;
    cv::applyColorMap(gray, colored, colorMapFromString(m_colorMap));
    frame.workingMat = colored;
    return {};
}
