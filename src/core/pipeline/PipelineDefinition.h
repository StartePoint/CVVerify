#pragma once

#include <QString>
#include <QVariantMap>
#include <QVector>

struct PipelineStepDefinition
{
    QString stepId;
    QString displayName;
    QVariantMap parameters;
};

struct PipelineDefinition
{
    QString pipelineId;
    QString displayName;
    QVector<PipelineStepDefinition> steps;
};
