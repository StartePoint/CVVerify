#pragma once

#include "core/pipeline/IPipelineStep.h"

class WatershedSegmentationOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_markerMode = "distance_transform_auto";
    int m_openingKernel = 3;
    int m_dilateIterations = 3;
    double m_distanceThresholdRatio = 0.35;
    int m_minMarkerArea = 32;
    QString m_boundaryColor = "#ff2d55";
};
