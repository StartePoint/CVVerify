#include "core/operators/OperatorSchemaExporter.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include "core/operators/OperatorRegistry.h"

namespace OperatorSchemaExporter {

namespace {

QString schemaFileStem(const QString& operatorId)
{
    const int dotIndex = operatorId.lastIndexOf('.');
    return dotIndex >= 0 ? operatorId.mid(dotIndex + 1) : operatorId;
}

void appendNumericField(QJsonObject* object, const char* key, const QVariant& value)
{
    if (value.isValid()) {
        object->insert(key, QJsonValue::fromVariant(value));
    }
}

}

QString parameterTypeToString(StepParameterType type)
{
    switch (type) {
    case StepParameterType::Integer:
        return "integer";
    case StepParameterType::Double:
        return "double";
    case StepParameterType::Boolean:
        return "boolean";
    case StepParameterType::Choice:
        return "choice";
    case StepParameterType::String:
    default:
        return "string";
    }
}

QJsonObject parameterToJson(const StepParameter& parameter)
{
    QJsonObject object;
    object.insert("key", parameter.key);
    object.insert("displayName", parameter.displayName);
    object.insert("group", parameter.group);
    object.insert("type", parameterTypeToString(parameter.type));
    if (parameter.defaultValue.isValid()) {
        object.insert("default", QJsonValue::fromVariant(parameter.defaultValue));
    }
    appendNumericField(&object, "min", parameter.minimumValue);
    appendNumericField(&object, "max", parameter.maximumValue);
    appendNumericField(&object, "step", parameter.stepValue);
    if (!parameter.visibleWhen.isEmpty()) {
        object.insert("visibleWhen", parameter.visibleWhen);
    }
    if (!parameter.tooltip.isEmpty()) {
        object.insert("tooltip", parameter.tooltip);
    }

    if (!parameter.choices.isEmpty()) {
        QJsonArray choices;
        for (const StepParameterChoice& choice : parameter.choices) {
            choices.append(QJsonObject{
                {"value", choice.value},
                {"label", choice.label},
            });
        }
        object.insert("choices", choices);
    }

    return object;
}

QJsonObject schemaToJson(const StepSchema& schema, const QString& category, const QString& description)
{
    QJsonObject root;
    root.insert("id", schema.id);
    root.insert("displayName", schema.displayName);
    if (!category.isEmpty()) {
        root.insert("category", category);
    }
    if (!description.isEmpty()) {
        root.insert("description", description);
    }

    QJsonArray parameters;
    for (const StepParameter& parameter : schema.parameters) {
        parameters.append(parameterToJson(parameter));
    }
    root.insert("parameters", parameters);
    return root;
}

bool writeSchemaFile(const StepSchema& schema, const QString& outputPath, QString* errorMessage)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write schema file: %1").arg(outputPath);
        }
        return false;
    }

    file.write(QJsonDocument(schemaToJson(schema)).toJson(QJsonDocument::Indented));
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int exportAllRegisteredSchemas(const OperatorRegistry& registry, const QString& outputDirectory, QStringList* errorMessages)
{
    QDir().mkpath(outputDirectory);

    int failureCount = 0;
    for (const OperatorDescriptor& descriptor : registry.allOperators()) {
        const std::shared_ptr<IPipelineStep> step = registry.create(descriptor.id);
        if (!step) {
            ++failureCount;
            if (errorMessages) {
                errorMessages->append(QString("%1: failed to create step").arg(descriptor.id));
            }
            continue;
        }

        const QString outputPath = QDir(outputDirectory).filePath(schemaFileStem(descriptor.id) + ".json");
        QString writeError;
        if (!writeSchemaFile(step->schema(), outputPath, &writeError)) {
            ++failureCount;
            if (errorMessages) {
                errorMessages->append(QString("%1: %2").arg(descriptor.id, writeError));
            }
        }
    }

    return failureCount;
}

}
