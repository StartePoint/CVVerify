#pragma once

#include "core/pipeline/IPipelineStep.h"

class BoxFilterOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_kernelWidth = 3;
    int m_kernelHeight = 3;
    bool m_normalize = true;
};
