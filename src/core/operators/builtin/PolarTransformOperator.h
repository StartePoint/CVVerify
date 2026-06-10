#pragma once

#include "core/pipeline/IPipelineStep.h"

class PolarTransformOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_direction = "cart_to_polar";
    QString m_polarMode = "linear";
    QString m_centerMode = "image_center";
    double m_centerX = 0.0;
    double m_centerY = 0.0;
    QString m_radiusMode = "auto";
    double m_maxRadius = 0.0;
    int m_outputWidth = 0;
    int m_outputHeight = 0;
    QString m_interpolation = "linear";
};
