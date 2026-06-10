#pragma once

#include "core/pipeline/IPipelineStep.h"

class CornerDetectOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_method = "shi_tomasi";
    int m_maxCorners = 200;
    double m_qualityLevel = 0.01;
    double m_minDistance = 10.0;
    int m_blockSize = 3;
    double m_harrisK = 0.04;
};
