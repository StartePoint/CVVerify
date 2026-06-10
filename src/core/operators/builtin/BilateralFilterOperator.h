#pragma once

#include "core/pipeline/IPipelineStep.h"

class BilateralFilterOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_diameter = 9;
    double m_sigmaColor = 75.0;
    double m_sigmaSpace = 75.0;
};
