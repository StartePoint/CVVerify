#pragma once

#include "core/pipeline/IPipelineStep.h"

class LinearAbsTransformOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_k = 1.0;
    double m_b = 0.0;
    bool m_applyAbs = true;
};
