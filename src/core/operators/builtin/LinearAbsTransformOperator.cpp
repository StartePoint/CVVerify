#include "core/operators/builtin/LinearAbsTransformOperator.h"

#include <opencv2/core.hpp>

QString LinearAbsTransformOperator::id() const
{
    return "builtin.linear_abs_transform";
}

QString LinearAbsTransformOperator::displayName() const
{
    return "Linear Abs Transform";
}

StepSchema LinearAbsTransformOperator::schema() const
{
    return {
        "builtin.linear_abs_transform",
        "Linear Abs Transform",
        {
            {"k", "Slope K", "变换设置", StepParameterType::Double, 1.0, -10.0, 10.0, 0.05, {}, {}, {}},
            {"b", "Bias B", "变换设置", StepParameterType::Double, 0.0, -255.0, 255.0, 1.0, {}, {}, {}},
            {"applyAbs", "Apply Abs", "变换设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap LinearAbsTransformOperator::parameterValues() const
{
    return {
        {"k", m_k},
        {"b", m_b},
        {"applyAbs", m_applyAbs},
    };
}

void LinearAbsTransformOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("k")) {
        m_k = values.value("k").toDouble();
    }
    if (values.contains("b")) {
        m_b = values.value("b").toDouble();
    }
    if (values.contains("applyAbs")) {
        m_applyAbs = values.value("applyAbs").toBool();
    }
}

StepResult LinearAbsTransformOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat transformed;
    frame.workingMat.convertTo(transformed, CV_32F, m_k, m_b);
    if (m_applyAbs) {
        cv::Mat absTransformed;
        cv::absdiff(transformed, cv::Scalar::all(0), absTransformed);
        transformed = absTransformed;
    }
    transformed.convertTo(frame.workingMat, frame.workingMat.type());
    return {};
}
