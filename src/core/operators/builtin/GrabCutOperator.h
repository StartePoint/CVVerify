#pragma once

#include "core/pipeline/IPipelineStep.h"

class GrabCutOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_initMode = "rect";
    int m_iterCount = 5;
    QString m_outputMode = "foreground_rgba";
    int m_featherRadius = 0;
};
