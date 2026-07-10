#pragma once

#include "core/pipeline/IPipelineStep.h"

class CropOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 640;
    int m_height = 480;
};
