#pragma once

#include "core/pipeline/IPipelineStep.h"

class AdaptiveThresholdOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_maxValue = 255.0;
    QString m_adaptiveMethod = "gaussian";
    QString m_thresholdType = "binary";
    int m_blockSize = 11;
    double m_cValue = 2.0;
};
