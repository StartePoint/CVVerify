#include "core/operators/builtin/BilateralFilterOperator.h"

#include <opencv2/imgproc.hpp>

QString BilateralFilterOperator::id() const
{
    return "builtin.bilateral_filter";
}

QString BilateralFilterOperator::displayName() const
{
    return "Bilateral Filter";
}

StepSchema BilateralFilterOperator::schema() const
{
    return {
        "builtin.bilateral_filter",
        "Bilateral Filter",
        {
            {"diameter", "Diameter", "滤波设置", StepParameterType::Integer, 9, 1, 31, 1, {}, {}, {}},
            {"sigmaColor", "Sigma Color", "滤波设置", StepParameterType::Double, 75.0, 1.0, 255.0, 1.0, {}, {}, {}},
            {"sigmaSpace", "Sigma Space", "滤波设置", StepParameterType::Double, 75.0, 1.0, 255.0, 1.0, {}, {}, {}},
        }
    };
}

QVariantMap BilateralFilterOperator::parameterValues() const
{
    return {
        {"diameter", m_diameter},
        {"sigmaColor", m_sigmaColor},
        {"sigmaSpace", m_sigmaSpace},
    };
}

void BilateralFilterOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("diameter")) {
        m_diameter = values.value("diameter").toInt();
    }
    if (values.contains("sigmaColor")) {
        m_sigmaColor = values.value("sigmaColor").toDouble();
    }
    if (values.contains("sigmaSpace")) {
        m_sigmaSpace = values.value("sigmaSpace").toDouble();
    }
}

StepResult BilateralFilterOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat filtered;
    cv::bilateralFilter(frame.workingMat, filtered, std::max(1, m_diameter), m_sigmaColor, m_sigmaSpace);
    frame.workingMat = filtered;
    return {};
}
