#pragma once

#include "core/pipeline/IPipelineStep.h"

class SobelOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_outputMode = "magnitude";
    int m_dx = 1;
    int m_dy = 1;
    int m_kernelSize = 3;
    double m_scale = 1.0;
    double m_delta = 0.0;
};
