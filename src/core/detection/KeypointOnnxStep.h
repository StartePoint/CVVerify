#pragma once

#include <QString>
#include <QStringList>

#include "core/detection/DetectionModelDescriptor.h"
#include "core/pipeline/IPipelineStep.h"

class KeypointOnnxStep : public IPipelineStep
{
public:
    KeypointOnnxStep(DetectionModelDescriptor descriptor, QString modelPath, QStringList labels);

    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    DetectionModelDescriptor m_descriptor;
    QString m_modelPath;
    QStringList m_labels;
    float m_scoreThreshold = 0.25f;
    int m_maxKeypointsPerChannel = 1;
};
