#pragma once

#include "core/pipeline/IPipelineStep.h"

class BrightnessContrastOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_alpha = 1.0;
    double m_beta = 0.0;
};
