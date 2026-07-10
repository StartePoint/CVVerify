#pragma once

#include "core/pipeline/IPipelineStep.h"

class RotateOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_angle = 0.0;
    double m_scale = 1.0;
    QString m_borderMode = "constant";
    int m_borderValue = 0;
};
