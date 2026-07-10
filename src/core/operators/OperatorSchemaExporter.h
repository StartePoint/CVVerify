#pragma once

#include <QString>

#include <QJsonObject>

#include "core/pipeline/IPipelineStep.h"

class OperatorRegistry;

namespace OperatorSchemaExporter {

QString parameterTypeToString(StepParameterType type);
QJsonObject parameterToJson(const StepParameter& parameter);
QJsonObject schemaToJson(const StepSchema& schema, const QString& category = QString(), const QString& description = QString());
bool writeSchemaFile(const StepSchema& schema, const QString& outputPath, QString* errorMessage = nullptr);
int exportAllRegisteredSchemas(const OperatorRegistry& registry, const QString& outputDirectory, QStringList* errorMessages = nullptr);

}
