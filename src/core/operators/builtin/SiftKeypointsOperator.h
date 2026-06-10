#pragma once

#include "core/pipeline/IPipelineStep.h"

class SiftKeypointsOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_nFeatures = 0;
    int m_nOctaveLayers = 3;
    double m_contrastThreshold = 0.04;
    double m_edgeThreshold = 10.0;
    double m_sigma = 1.6;
    bool m_drawRichKeypoints = true;
    int m_maxDisplayCount = 300;
};
