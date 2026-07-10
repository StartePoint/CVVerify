#include "core/operators/builtin/CropOperator.h"

#include <opencv2/imgproc.hpp>

QString CropOperator::id() const
{
    return "builtin.crop";
}

QString CropOperator::displayName() const
{
    return "Crop";
}

StepSchema CropOperator::schema() const
{
    return {
        "builtin.crop",
        "Crop",
        {
            {"x", "X", "裁剪设置", StepParameterType::Integer, 0, 0, 8192, 1, {}, {}, {}},
            {"y", "Y", "裁剪设置", StepParameterType::Integer, 0, 0, 8192, 1, {}, {}, {}},
            {"width", "Width", "裁剪设置", StepParameterType::Integer, 640, 1, 8192, 1, {}, {}, {}},
            {"height", "Height", "裁剪设置", StepParameterType::Integer, 480, 1, 8192, 1, {}, {}, {}},
        }
    };
}

QVariantMap CropOperator::parameterValues() const
{
    return {
        {"x", m_x},
        {"y", m_y},
        {"width", m_width},
        {"height", m_height},
    };
}

void CropOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("x")) {
        m_x = values.value("x").toInt();
    }
    if (values.contains("y")) {
        m_y = values.value("y").toInt();
    }
    if (values.contains("width")) {
        m_width = values.value("width").toInt();
    }
    if (values.contains("height")) {
        m_height = values.value("height").toInt();
    }
}

StepResult CropOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (m_width <= 0 || m_height <= 0) {
        return {false, "Crop width and height must be positive"};
    }

    const cv::Rect cropRect(m_x, m_y, m_width, m_height);
    const cv::Rect imageRect(0, 0, frame.workingMat.cols, frame.workingMat.rows);
    const cv::Rect safeRect = cropRect & imageRect;
    if (safeRect.width <= 0 || safeRect.height <= 0) {
        return {false, "Crop region is outside the image bounds"};
    }

    frame.workingMat = frame.workingMat(safeRect).clone();
    return {};
}
