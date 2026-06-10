#pragma once

#include "core/pipeline/IPipelineStep.h"

class HistogramBarOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_channels = "gray";
    int m_bins = 256;
    double m_rangeMin = 0.0;
    double m_rangeMax = 255.0;
    bool m_normalize = true;
    QString m_barStyle = "filled";
};
