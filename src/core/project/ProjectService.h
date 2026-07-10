#pragma once

#include <QString>

#include "core/project/ProjectDefinition.h"

namespace ProjectService {

bool saveToFile(const ProjectDefinition& project, const QString& filePath, QString* errorMessage = nullptr);
ProjectLoadResult loadFromFile(const QString& filePath);

}
