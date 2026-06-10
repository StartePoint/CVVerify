#pragma once

#include "core/pipeline/IPipelineStep.h"

class HistogramEqualizeOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_mode = "global";
    QString m_colorMode = "gray_or_luminance";
    double m_clipLimit = 2.0;
    int m_tileGridWidth = 8;
    int m_tileGridHeight = 8;
};
