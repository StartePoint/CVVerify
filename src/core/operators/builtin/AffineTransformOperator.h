#pragma once

#include "core/pipeline/IPipelineStep.h"

class AffineTransformOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_outputWidth = 1024;
    int m_outputHeight = 768;
    QString m_interpolation = "linear";
    QString m_borderType = "constant";
    int m_borderValue = 0;
};
