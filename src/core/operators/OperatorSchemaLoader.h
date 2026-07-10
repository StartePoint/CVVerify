#pragma once

#include <QString>
#include <QStringList>

#include <QVector>

#include "core/pipeline/IPipelineStep.h"

class OperatorRegistry;

struct OperatorSchemaDocument
{
    QString id;
    QString displayName;
    QString category;
    QString description;
    StepSchema schema;
};

namespace OperatorSchemaLoader {

QString schemasDirectory();
QStringList listSchemas();
QStringList indexedSchemaNames();
bool loadSchema(const QString& schemaName, OperatorSchemaDocument* document, QString* errorMessage = nullptr);
bool validateAgainstStep(const OperatorSchemaDocument& document, const IPipelineStep& step, QString* errorMessage = nullptr);
int validateIndexedSchemas(const OperatorRegistry& registry, QStringList* failureMessages = nullptr);

}
