#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "app/AppSettings.h"
#include "core/detection/DetectionModelDescriptor.h"
#include "core/pipeline/PipelineDefinition.h"

struct ProjectDefinition
{
    QString projectId;
    QString displayName;
    QString version = "1.0";
    QString mediaSourcePath;
    QString mediaSourceKind;
    int currentVideoFrameIndex = 0;
    PipelineDefinition pipeline;
    DetectionModelDescriptor activeDetectionModel;
    QString activeDetectionModelPath;
    QString activeDetectionModelPackageDir;
    AppSettings appSettings;
    QStringList recentMediaFiles;
};

struct ProjectLoadResult
{
    bool success = false;
    QString errorMessage;
    ProjectDefinition project;
};
