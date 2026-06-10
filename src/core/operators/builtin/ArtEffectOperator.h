#pragma once

#include "core/pipeline/IPipelineStep.h"

class ArtEffectOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_styleMode = "stylization";
    double m_sigmaS = 60.0;
    double m_sigmaR = 0.45;
};
