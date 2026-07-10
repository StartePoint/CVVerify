#pragma once

#include "core/pipeline/IPipelineStep.h"

class TemplateMatchOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_templateImagePath;
    QString m_matchMethod = "ccoeff_normed";
    double m_threshold = 0.8;
};
