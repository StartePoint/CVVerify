#pragma once

#include "core/pipeline/IPipelineStep.h"

class FiberGlowOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_angleDeg = 25.0;
    int m_streakLength = 15;
    double m_intensity = 1.2;
    double m_threshold = 180.0;
    double m_blendAlpha = 0.7;
};
