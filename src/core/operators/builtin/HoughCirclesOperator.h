#pragma once

#include "core/pipeline/IPipelineStep.h"

class HoughCirclesOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_dp = 1.0;
    double m_minDist = 20.0;
    double m_param1 = 100.0;
    double m_param2 = 30.0;
    int m_minRadius = 0;
    int m_maxRadius = 0;
};
