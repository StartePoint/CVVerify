#pragma once

#include "core/pipeline/IPipelineStep.h"

class CannyOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_threshold1 = 50.0;
    double m_threshold2 = 150.0;
    int m_apertureSize = 3;
    bool m_useL2Gradient = false;
};
