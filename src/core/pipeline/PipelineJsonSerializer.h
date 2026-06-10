#pragma once

#include <QString>

#include "core/pipeline/PipelineDefinition.h"

struct PipelineLoadResult
{
    bool success = false;
    QString errorMessage;
    PipelineDefinition definition;
};

namespace PipelineJsonSerializer {

bool saveToFile(const PipelineDefinition& definition, const QString& filePath, QString* errorMessage = nullptr);
PipelineLoadResult loadFromFile(const QString& filePath);

}
