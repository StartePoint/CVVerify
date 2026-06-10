#pragma once

#include "core/pipeline/IPipelineStep.h"

class ContourDetectOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_retrievalMode = "external";
    QString m_approxMode = "simple";
    double m_minArea = 10.0;
    double m_maxArea = 0.0;
    QString m_drawMode = "outline";
    int m_thickness = 2;
};
