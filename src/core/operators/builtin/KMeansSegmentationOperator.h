#pragma once

#include "core/pipeline/IPipelineStep.h"

class KMeansSegmentationOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_clusterCount = 4;
    QString m_colorSpace = "lab";
    bool m_includeSpatialXY = false;
    double m_spatialWeight = 1.0;
    int m_maxIterations = 20;
    double m_epsilon = 1.0;
    int m_attempts = 3;
};
