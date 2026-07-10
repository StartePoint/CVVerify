#include "core/project/ParameterPresetStore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "infra/platform/PlatformPaths.h"

namespace ParameterPresetStore {

namespace {

QString presetFilePath(const QString& operatorId, const QString& presetName)
{
    QString safeOperatorId = operatorId;
    QString safePresetName = presetName;
    safeOperatorId.replace('/', '_');
    safePresetName.replace('/', '_');
    return QDir(PlatformPaths::presetsDirectory()).filePath(QString("%1__%2.json").arg(safeOperatorId, safePresetName));
}

QVariantMap readJsonObjectFile(const QString& filePath, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open preset: %1").arg(filePath);
        }
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid preset JSON: %1").arg(filePath);
        }
        return {};
    }

    return document.object().toVariantMap();
}

bool writeJsonObjectFile(const QString& filePath, const QVariantMap& values, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write preset: %1").arg(filePath);
        }
        return false;
    }

    const QJsonDocument document(QJsonObject::fromVariantMap(values));
    file.write(document.toJson(QJsonDocument::Indented));
    return true;
}

}

bool savePreset(const QString& operatorId, const QString& presetName, const QVariantMap& values, QString* errorMessage)
{
    if (operatorId.trimmed().isEmpty() || presetName.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Operator id and preset name are required";
        }
        return false;
    }

    return writeJsonObjectFile(presetFilePath(operatorId, presetName), values, errorMessage);
}

bool loadPreset(const QString& operatorId, const QString& presetName, QVariantMap* values, QString* errorMessage)
{
    if (!values) {
        return false;
    }

    const QVariantMap loaded = readJsonObjectFile(presetFilePath(operatorId, presetName), errorMessage);
    if (loaded.isEmpty() && errorMessage && !errorMessage->isEmpty()) {
        return false;
    }

    *values = loaded;
    return true;
}

QStringList listPresets(const QString& operatorId)
{
    QString safeOperatorId = operatorId;
    safeOperatorId.replace('/', '_');
    const QString prefix = safeOperatorId + "__";
    QStringList names;
    const QFileInfoList files = QDir(PlatformPaths::presetsDirectory()).entryInfoList({"*.json"}, QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        const QString baseName = fileInfo.baseName();
        if (baseName.startsWith(prefix)) {
            names.append(baseName.mid(prefix.size()));
        }
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}

bool removePreset(const QString& operatorId, const QString& presetName, QString* errorMessage)
{
    const QString filePath = presetFilePath(operatorId, presetName);
    if (!QFile::exists(filePath)) {
        if (errorMessage) {
            *errorMessage = QString("Preset not found: %1").arg(presetName);
        }
        return false;
    }

    if (!QFile::remove(filePath)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to remove preset: %1").arg(presetName);
        }
        return false;
    }

    return true;
}

}
