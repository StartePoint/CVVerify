#pragma once

#include "core/pipeline/IPipelineStep.h"

class PencilSketchOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_outputMode = "gray";
    double m_sigmaS = 60.0;
    double m_sigmaR = 0.07;
    double m_shadeFactor = 0.02;
};
