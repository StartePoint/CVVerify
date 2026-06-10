#pragma once

#include "core/pipeline/IPipelineStep.h"

class HoughLinesOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_mode = "probabilistic";
    double m_rho = 1.0;
    double m_thetaDeg = 1.0;
    int m_threshold = 80;
    double m_minLineLength = 30.0;
    double m_maxLineGap = 10.0;
};
