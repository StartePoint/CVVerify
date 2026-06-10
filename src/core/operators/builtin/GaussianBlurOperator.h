#pragma once

#include "core/pipeline/IPipelineStep.h"

class GaussianBlurOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_kernelWidth = 5;
    int m_kernelHeight = 5;
    double m_sigmaX = 1.0;
    double m_sigmaY = 0.0;
};
