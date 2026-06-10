#pragma once

#include "core/pipeline/IPipelineStep.h"

class ColorMapOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_colorMap = "turbo";
    bool m_convertToGray = true;
    bool m_invertGray = false;
};
