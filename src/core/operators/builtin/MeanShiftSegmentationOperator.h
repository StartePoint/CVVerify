#pragma once

#include "core/pipeline/IPipelineStep.h"

class MeanShiftSegmentationOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    double m_spatialRadius = 10.0;
    double m_colorRadius = 20.0;
    int m_maxLevel = 1;
    int m_termMaxCount = 5;
    double m_termEpsilon = 1.0;
};
