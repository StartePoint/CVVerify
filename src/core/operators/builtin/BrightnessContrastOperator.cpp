#include "core/operators/builtin/BrightnessContrastOperator.h"

QString BrightnessContrastOperator::id() const
{
    return "builtin.brightness_contrast";
}

QString BrightnessContrastOperator::displayName() const
{
    return "Brightness Contrast";
}

StepSchema BrightnessContrastOperator::schema() const
{
    return {
        "builtin.brightness_contrast",
        "Brightness Contrast",
        {
            {"alpha", "Contrast Alpha", "增强设置", StepParameterType::Double, 1.0, 0.0, 5.0, 0.05, {}, {}, {}},
            {"beta", "Brightness Beta", "增强设置", StepParameterType::Double, 0.0, -255.0, 255.0, 1.0, {}, {}, {}},
        }
    };
}

QVariantMap BrightnessContrastOperator::parameterValues() const
{
    return {
        {"alpha", m_alpha},
        {"beta", m_beta},
    };
}

void BrightnessContrastOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("alpha")) {
        m_alpha = values.value("alpha").toDouble();
    }
    if (values.contains("beta")) {
        m_beta = values.value("beta").toDouble();
    }
}

StepResult BrightnessContrastOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    frame.workingMat.convertTo(frame.workingMat, -1, m_alpha, m_beta);
    return {};
}
