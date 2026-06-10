#include "core/pipeline/PipelineJsonSerializer.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace PipelineJsonSerializer {

bool saveToFile(const PipelineDefinition& definition, const QString& filePath, QString* errorMessage)
{
    QJsonObject root;
    root.insert("pipelineId", definition.pipelineId);
    root.insert("displayName", definition.displayName);

    QJsonArray stepsArray;
    for (const PipelineStepDefinition& step : definition.steps) {
        QJsonObject stepObject;
        stepObject.insert("stepId", step.stepId);
        stepObject.insert("displayName", step.displayName);
        stepObject.insert("parameters", QJsonObject::fromVariantMap(step.parameters));
        stepsArray.append(stepObject);
    }
    root.insert("steps", stepsArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open file for writing: %1").arg(filePath);
        }
        return false;
    }

    const QJsonDocument document(root);
    if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write pipeline file: %1").arg(filePath);
        }
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

PipelineLoadResult loadFromFile(const QString& filePath)
{
    PipelineLoadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = QString("Failed to open pipeline file: %1").arg(filePath);
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.errorMessage = QString("Failed to parse pipeline file: %1").arg(filePath);
        return result;
    }

    const QJsonObject root = document.object();
    result.definition.pipelineId = root.value("pipelineId").toString();
    result.definition.displayName = root.value("displayName").toString();

    const QJsonArray stepsArray = root.value("steps").toArray();
    for (const QJsonValue& value : stepsArray) {
        const QJsonObject stepObject = value.toObject();

        PipelineStepDefinition step;
        step.stepId = stepObject.value("stepId").toString();
        step.displayName = stepObject.value("displayName").toString();
        step.parameters = stepObject.value("parameters").toObject().toVariantMap();
        result.definition.steps.append(step);
    }

    result.success = true;
    return result;
}

}
