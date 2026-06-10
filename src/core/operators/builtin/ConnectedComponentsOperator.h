#pragma once

#include "core/pipeline/IPipelineStep.h"

class ConnectedComponentsOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_connectivity = 8;
    int m_minArea = 1;
    int m_maxArea = 0;
    bool m_colorize = true;
    bool m_drawBBox = true;
};
