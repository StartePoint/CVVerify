#pragma once

#include "core/pipeline/IPipelineStep.h"

class EmbossOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_direction = "north_west";
    double m_strength = 1.0;
    double m_bias = 128.0;
    bool m_convertToGray = false;
};
