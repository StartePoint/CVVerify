#include "core/operators/builtin/ResizeOperator.h"

#include <opencv2/imgproc.hpp>

QString ResizeOperator::id() const
{
    return "builtin.resize";
}

QString ResizeOperator::displayName() const
{
    return "Resize";
}

StepSchema ResizeOperator::schema() const
{
    return {
        "builtin.resize",
        "Resize",
        {
            {"width", "Width", "输出设置", StepParameterType::Integer, 640, 1, 8192, 1, {}, {}, {}},
            {"height", "Height", "输出设置", StepParameterType::Integer, 480, 1, 8192, 1, {}, {}, {}},
        }
    };
}

QVariantMap ResizeOperator::parameterValues() const
{
    return {
        {"width", m_width},
        {"height", m_height},
    };
}

void ResizeOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("width")) {
        m_width = values.value("width").toInt();
    }
    if (values.contains("height")) {
        m_height = values.value("height").toInt();
    }
}

StepResult ResizeOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat resized;
    cv::resize(frame.workingMat, resized, cv::Size(m_width, m_height));
    frame.workingMat = resized;
    return {};
}
