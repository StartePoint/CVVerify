#include "app/AppDefaultsLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "infra/platform/PlatformPaths.h"

namespace AppDefaultsLoader {

namespace {

QString readString(const QJsonObject& object, const QString& key, const QString& fallback = QString())
{
    return object.contains(key) ? object.value(key).toString() : fallback;
}

int readInt(const QJsonObject& object, const QString& key, int fallback)
{
    return object.contains(key) ? object.value(key).toInt(fallback) : fallback;
}

double readDouble(const QJsonObject& object, const QString& key, double fallback)
{
    return object.contains(key) ? object.value(key).toDouble(fallback) : fallback;
}

bool readBool(const QJsonObject& object, const QString& key, bool fallback)
{
    return object.contains(key) ? object.value(key).toBool(fallback) : fallback;
}

void applyPreviewDefaults(const QJsonObject& preview, AppSettings* settings)
{
    if (preview.isEmpty() || !settings) {
        return;
    }

    settings->maxPreviewFps = readInt(preview, "maxFps", settings->maxPreviewFps);
    settings->allowFrameSkip = readBool(preview, "allowFrameSkip", settings->allowFrameSkip);
    settings->previewScale = readDouble(preview, "previewScale", settings->previewScale);
    settings->previewFrameStep = readInt(preview, "frameStep", settings->previewFrameStep);
}

void applyTaskDefaults(const QJsonObject& task, AppSettings* settings)
{
    if (task.isEmpty() || !settings) {
        return;
    }

    settings->offlineVideoStartFrame = readInt(task, "offlineVideoStartFrame", settings->offlineVideoStartFrame);
    settings->offlineVideoEndFrame = readInt(task, "offlineVideoEndFrame", settings->offlineVideoEndFrame);
}

void applyDnnDefaults(const QJsonObject& dnn, AppSettings* settings)
{
    if (dnn.isEmpty() || !settings) {
        return;
    }

    const QString backend = readString(dnn, "backend");
    if (!backend.isEmpty()) {
        settings->dnnBackend = backend;
    }
}

void applyExportDefaults(const QJsonObject& exportObject, AppSettings* settings)
{
    if (exportObject.isEmpty() || !settings) {
        return;
    }

    settings->exportVideoSideBySide = readBool(
        exportObject,
        "videoSideBySide",
        settings->exportVideoSideBySide);
}

void applyUiDefaults(const QJsonObject& ui, AppSettings* settings)
{
    if (ui.isEmpty() || !settings) {
        return;
    }

    settings->showWelcomeGuide = readBool(ui, "showWelcomeGuide", settings->showWelcomeGuide);
}

}

bool applyTo(AppSettings* settings, QString* errorMessage)
{
    if (!settings) {
        if (errorMessage) {
            *errorMessage = "App settings output is null";
        }
        return false;
    }

    const QString filePath = PlatformPaths::resolveResourcePath("config/app_defaults.json");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open app defaults: %1").arg(filePath);
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid app defaults JSON: %1").arg(filePath);
        }
        return false;
    }

    const QJsonObject root = document.object();
    const QString languageCode = readString(root, "languageCode");
    if (!languageCode.isEmpty()) {
        settings->languageCode = languageCode;
    }

    settings->defaultExportDirectory = readString(root, "defaultExportDirectory", settings->defaultExportDirectory);
    settings->autoOpenExportDirectory = readBool(root, "autoOpenExportDirectory", settings->autoOpenExportDirectory);
    settings->maxRecentMediaFiles = readInt(root, "maxRecentMediaFiles", settings->maxRecentMediaFiles);

    applyPreviewDefaults(root.value("preview").toObject(), settings);
    applyTaskDefaults(root.value("task").toObject(), settings);
    applyDnnDefaults(root.value("dnn").toObject(), settings);
    applyExportDefaults(root.value("export").toObject(), settings);
    applyUiDefaults(root.value("ui").toObject(), settings);

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

}
