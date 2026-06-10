#pragma once

#include "core/pipeline/IPipelineStep.h"

class SurfMatchPlaceholderOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    QString m_secondImageSource = "external_pick";
    QString m_secondaryImagePath;
    QString m_backend = "sift";
    QString m_matcherType = "bf";
    double m_ratioTest = 0.75;
    bool m_crossCheck = false;
    int m_maxMatches = 80;
    bool m_useRansac = true;
    double m_ransacReprojThreshold = 3.0;
    bool m_showInliersOnly = false;
};
