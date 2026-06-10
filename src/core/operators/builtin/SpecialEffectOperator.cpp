#include "core/operators/builtin/SpecialEffectOperator.h"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureColorForSpecialEffect(const cv::Mat& source)
{
    if (source.channels() == 3) {
        return source;
    }

    cv::Mat color;
    cv::cvtColor(source, color, cv::COLOR_GRAY2BGR);
    return color;
}

int oddRadius(int value)
{
    int radius = std::max(1, value);
    if (radius % 2 == 0) {
        radius += 1;
    }
    return radius;
}

cv::Mat posterize(const cv::Mat& color, int levels)
{
    cv::Mat output = color.clone();
    const int bucketSize = std::max(1, 256 / std::max(2, levels));
    output.forEach<cv::Vec3b>([bucketSize](cv::Vec3b& pixel, const int*) {
        for (int index = 0; index < 3; ++index) {
            pixel[index] = static_cast<uchar>((pixel[index] / bucketSize) * bucketSize + bucketSize / 2);
        }
    });
    return output;
}
}

QString SpecialEffectOperator::id() const
{
    return "builtin.special_effect";
}

QString SpecialEffectOperator::displayName() const
{
    return "Special Effect";
}

StepSchema SpecialEffectOperator::schema() const
{
    return {
        "builtin.special_effect",
        "Special Effect",
        {
            {"preset", "Preset", "效果设置", StepParameterType::Choice, "cartoon", {}, {}, {}, {}, {}, {
                {"cartoon", "Cartoon"},
                {"oil_paint", "Oil Paint"},
                {"posterize", "Posterize"},
                {"negative", "Negative"},
                {"frosted_glass", "Frosted Glass"},
            }},
            {"strength", "Strength", "效果设置", StepParameterType::Double, 1.0, 0.0, 10.0, 0.1, {}, {}, {}},
            {"radius", "Radius", "效果设置", StepParameterType::Integer, 5, 1, 51, 1, {}, {}, {}},
        }
    };
}

QVariantMap SpecialEffectOperator::parameterValues() const
{
    return {
        {"preset", m_preset},
        {"strength", m_strength},
        {"radius", m_radius},
    };
}

void SpecialEffectOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("preset")) m_preset = values.value("preset").toString();
    if (values.contains("strength")) m_strength = values.value("strength").toDouble();
    if (values.contains("radius")) m_radius = values.value("radius").toInt();
}

StepResult SpecialEffectOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat color = ensureColorForSpecialEffect(frame.workingMat);
    cv::Mat output;

    if (m_preset == "negative") {
        cv::bitwise_not(color, output);
    } else if (m_preset == "posterize") {
        const int levels = std::max(2, 10 - static_cast<int>(std::round(std::min(8.0, m_strength))));
        output = posterize(color, levels);
    } else if (m_preset == "oil_paint") {
        cv::medianBlur(color, output, oddRadius(m_radius));
        output = posterize(output, std::max(3, 12 - static_cast<int>(std::round(std::min(8.0, m_strength)))));
    } else if (m_preset == "frosted_glass") {
        output = color.clone();
        cv::RNG rng(12345);
        const int radius = std::max(1, m_radius);
        for (int row = 0; row < color.rows; ++row) {
            for (int col = 0; col < color.cols; ++col) {
                const int offsetY = rng.uniform(-radius, radius + 1);
                const int offsetX = rng.uniform(-radius, radius + 1);
                const int sampleRow = std::clamp(row + offsetY, 0, color.rows - 1);
                const int sampleCol = std::clamp(col + offsetX, 0, color.cols - 1);
                output.at<cv::Vec3b>(row, col) = color.at<cv::Vec3b>(sampleRow, sampleCol);
            }
        }
    } else {
        cv::Mat smoothed;
        cv::bilateralFilter(color, smoothed, 9, 40.0 * std::max(1.0, m_strength), 20.0 * std::max(1.0, m_strength));
        cv::Mat gray;
        cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);
        cv::medianBlur(gray, gray, oddRadius(m_radius));
        cv::Mat edges;
        cv::adaptiveThreshold(gray, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 9, 2);
        cv::cvtColor(edges, edges, cv::COLOR_GRAY2BGR);
        cv::bitwise_and(smoothed, edges, output);
    }

    frame.workingMat = output;
    return {};
}
