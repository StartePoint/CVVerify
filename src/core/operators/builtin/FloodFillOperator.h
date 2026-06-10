#pragma once

#include "core/pipeline/IPipelineStep.h"

class FloodFillOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_seedMode = "manual_click";
    int m_connectivity = 4;
    int m_loDiff = 20;
    int m_upDiff = 20;
    bool m_fixedRange = false;
    QString m_outputMode = "overlay";
    QString m_fillColor = "#00ff99";
};
