#include "core/operators/OperatorSchemaLoader.h"

#include <memory>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/operators/OperatorRegistry.h"
#include "infra/platform/PlatformPaths.h"

namespace OperatorSchemaLoader {

namespace {

StepParameterType parameterTypeFromString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == "integer") {
        return StepParameterType::Integer;
    }
    if (normalized == "double") {
        return StepParameterType::Double;
    }
    if (normalized == "boolean") {
        return StepParameterType::Boolean;
    }
    if (normalized == "choice") {
        return StepParameterType::Choice;
    }
    return StepParameterType::String;
}

QString readString(const QJsonObject& object, const QString& camelKey, const QString& snakeKey, const QString& fallback = QString())
{
    if (object.contains(camelKey)) {
        return object.value(camelKey).toString();
    }
    if (object.contains(snakeKey)) {
        return object.value(snakeKey).toString();
    }
    return fallback;
}

StepParameter parameterFromJson(const QJsonObject& object)
{
    StepParameter parameter;
    parameter.key = readString(object, "key", "key");
    parameter.displayName = readString(object, "displayName", "display_name");
    parameter.group = readString(object, "group", "group");
    parameter.type = parameterTypeFromString(readString(object, "type", "type", "string"));
    parameter.defaultValue = object.value("default").toVariant();
    parameter.minimumValue = object.contains("min") ? object.value("min").toVariant() : object.value("minimum").toVariant();
    parameter.maximumValue = object.contains("max") ? object.value("max").toVariant() : object.value("maximum").toVariant();
    parameter.stepValue = object.contains("step") ? object.value("step").toVariant() : QVariant();
    parameter.visibleWhen = readString(object, "visibleWhen", "visible_when");
    parameter.tooltip = readString(object, "tooltip", "tooltip");

    const QJsonArray choices = object.value("choices").toArray();
    for (const QJsonValue& choiceValue : choices) {
        if (choiceValue.isObject()) {
            const QJsonObject choiceObject = choiceValue.toObject();
            parameter.choices.append({
                readString(choiceObject, "value", "value"),
                readString(choiceObject, "label", "label"),
            });
        } else if (choiceValue.isString()) {
            parameter.choices.append({choiceValue.toString(), choiceValue.toString()});
        }
    }

    return parameter;
}

}

QString schemasDirectory()
{
    return PlatformPaths::resolveResourcePath("config/operator_schemas");
}

QStringList listSchemas()
{
    const QDir directory(schemasDirectory());
    const QFileInfoList files = directory.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Name);
    QStringList names;
    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.completeBaseName() == "index") {
            continue;
        }
        names.append(fileInfo.completeBaseName());
    }
    return names;
}

QStringList indexedSchemaNames()
{
    const QString indexPath = QDir(schemasDirectory()).filePath("index.json");
    QFile file(indexPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return listSchemas();
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return listSchemas();
    }

    QStringList names;
    const QJsonArray schemas = document.object().value("schemas").toArray();
    for (const QJsonValue& value : schemas) {
        const QString name = value.toString().trimmed();
        if (!name.isEmpty()) {
            names.append(name);
        }
    }
    return names.isEmpty() ? listSchemas() : names;
}

bool loadSchema(const QString& schemaName, OperatorSchemaDocument* document, QString* errorMessage)
{
    if (!document) {
        if (errorMessage) {
            *errorMessage = "Operator schema output is null";
        }
        return false;
    }

    const QString trimmedName = schemaName.trimmed();
    if (trimmedName.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Operator schema name is empty";
        }
        return false;
    }

    const QString filePath = QDir(schemasDirectory()).filePath(trimmedName + ".json");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open operator schema: %1").arg(filePath);
        }
        return false;
    }

    const QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll());
    if (!jsonDocument.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid operator schema JSON: %1").arg(filePath);
        }
        return false;
    }

    const QJsonObject root = jsonDocument.object();
    document->id = readString(root, "id", "id");
    document->displayName = readString(root, "displayName", "display_name");
    document->category = readString(root, "category", "category");
    document->description = readString(root, "description", "description");

    StepSchema schema;
    schema.id = document->id;
    schema.displayName = document->displayName.isEmpty() ? document->id : document->displayName;

    const QJsonArray parameters = root.value("parameters").toArray();
    for (const QJsonValue& parameterValue : parameters) {
        if (!parameterValue.isObject()) {
            continue;
        }
        schema.parameters.append(parameterFromJson(parameterValue.toObject()));
    }

    document->schema = schema;
    return true;
}

bool validateAgainstStep(const OperatorSchemaDocument& document, const IPipelineStep& step, QString* errorMessage)
{
    if (document.id != step.id()) {
        if (errorMessage) {
            *errorMessage = QString("Schema id %1 does not match step id %2").arg(document.id, step.id());
        }
        return false;
    }

    const StepSchema runtimeSchema = step.schema();
    if (document.schema.parameters.size() != runtimeSchema.parameters.size()) {
        if (errorMessage) {
            *errorMessage = QString("Schema parameter count mismatch for %1").arg(step.id());
        }
        return false;
    }

    for (int index = 0; index < runtimeSchema.parameters.size(); ++index) {
        if (document.schema.parameters.at(index).key != runtimeSchema.parameters.at(index).key) {
            if (errorMessage) {
                *errorMessage = QString("Schema parameter key mismatch at index %1 for %2")
                    .arg(index)
                    .arg(step.id());
            }
            return false;
        }
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

int validateIndexedSchemas(const OperatorRegistry& registry, QStringList* failureMessages)
{
    int failureCount = 0;
    for (const QString& schemaName : indexedSchemaNames()) {
        OperatorSchemaDocument document;
        QString loadError;
        if (!loadSchema(schemaName, &document, &loadError)) {
            ++failureCount;
            if (failureMessages) {
                failureMessages->append(QString("%1: %2").arg(schemaName, loadError));
            }
            continue;
        }

        const std::shared_ptr<IPipelineStep> step = registry.create(document.id);
        if (!step) {
            ++failureCount;
            if (failureMessages) {
                failureMessages->append(QString("%1: operator not registered (%2)").arg(schemaName, document.id));
            }
            continue;
        }

        QString validateError;
        if (!validateAgainstStep(document, *step, &validateError)) {
            ++failureCount;
            if (failureMessages) {
                failureMessages->append(QString("%1: %2").arg(schemaName, validateError));
            }
        }
    }

    return failureCount;
}

}
