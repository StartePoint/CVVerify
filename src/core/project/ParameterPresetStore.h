#pragma once

#include <QVariantMap>

#include <QString>

namespace ParameterPresetStore {

bool savePreset(const QString& operatorId, const QString& presetName, const QVariantMap& values, QString* errorMessage = nullptr);
bool loadPreset(const QString& operatorId, const QString& presetName, QVariantMap* values, QString* errorMessage = nullptr);
QStringList listPresets(const QString& operatorId);
bool removePreset(const QString& operatorId, const QString& presetName, QString* errorMessage = nullptr);

}
