#include "core/operators/builtin/BoxFilterOperator.h"

#include <opencv2/imgproc.hpp>

QString BoxFilterOperator::id() const
{
    return "builtin.box_filter";
}

QString BoxFilterOperator::displayName() const
{
    return "Box Filter";
}

StepSchema BoxFilterOperator::schema() const
{
    return {
        "builtin.box_filter",
        "Box Filter",
        {
            {"kernelWidth", "Kernel Width", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
            {"kernelHeight", "Kernel Height", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
            {"normalize", "Normalize", "输出设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap BoxFilterOperator::parameterValues() const
{
    return {
        {"kernelWidth", m_kernelWidth},
        {"kernelHeight", m_kernelHeight},
        {"normalize", m_normalize},
    };
}

void BoxFilterOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("kernelWidth")) {
        m_kernelWidth = values.value("kernelWidth").toInt();
    }
    if (values.contains("kernelHeight")) {
        m_kernelHeight = values.value("kernelHeight").toInt();
    }
    if (values.contains("normalize")) {
        m_normalize = values.value("normalize").toBool();
    }
}

StepResult BoxFilterOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat filtered;
    cv::boxFilter(
        frame.workingMat,
        filtered,
        -1,
        cv::Size(std::max(1, m_kernelWidth), std::max(1, m_kernelHeight)),
        cv::Point(-1, -1),
        m_normalize
    );
    frame.workingMat = filtered;
    return {};
}
