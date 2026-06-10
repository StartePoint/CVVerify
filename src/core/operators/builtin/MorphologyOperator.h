#pragma once

#include "core/pipeline/IPipelineStep.h"

class MorphologyOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_operation = "dilate";
    QString m_kernelShape = "rect";
    int m_kernelWidth = 3;
    int m_kernelHeight = 3;
    int m_iterations = 1;
    QString m_borderType = "reflect101";
    int m_borderValue = 0;
};
