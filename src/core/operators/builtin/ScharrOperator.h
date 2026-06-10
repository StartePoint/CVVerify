#pragma once

#include "core/pipeline/IPipelineStep.h"

class ScharrOperator : public IPipelineStep
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
    double m_scale = 1.0;
    double m_delta = 0.0;
};
