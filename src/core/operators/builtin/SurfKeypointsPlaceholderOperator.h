#pragma once

#include "core/pipeline/IPipelineStep.h"

class SurfKeypointsPlaceholderOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_backend = "sift";
    double m_hessianThreshold = 400.0;
    int m_nOctaves = 4;
    int m_nOctaveLayers = 3;
    bool m_extended = false;
    bool m_upright = false;
    int m_maxDisplayCount = 300;
};
