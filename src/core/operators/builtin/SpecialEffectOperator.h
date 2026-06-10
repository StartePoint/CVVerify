#pragma once

#include "core/pipeline/IPipelineStep.h"

class SpecialEffectOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_preset = "cartoon";
    double m_strength = 1.0;
    int m_radius = 5;
};
