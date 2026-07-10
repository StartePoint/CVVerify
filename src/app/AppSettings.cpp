#include "app/AppSettings.h"
#include "app/AppDefaultsLoader.h"

#include <QSettings>

namespace {

const char* const kOrganizationName = "StartePoint";
const char* const kApplicationName = "CVVerify";

}

namespace AppSettingsStore {

AppSettings load()
{
    AppSettings appSettings;
    AppDefaultsLoader::applyTo(&appSettings);

    QSettings settings(kOrganizationName, kApplicationName);
    appSettings.languageCode = settings.value("ui/languageCode", appSettings.languageCode).toString();
    appSettings.defaultExportDirectory = settings.value("export/defaultDirectory", QString()).toString();
    appSettings.autoOpenExportDirectory = settings.value("export/autoOpenDirectory", false).toBool();
    appSettings.recentMediaFiles = settings.value("media/recentFiles", QStringList()).toStringList();
    appSettings.recentProjectFiles = settings.value("project/recentFiles", QStringList()).toStringList();
    appSettings.maxRecentMediaFiles = settings.value("media/maxRecentFiles", appSettings.maxRecentMediaFiles).toInt();
    appSettings.maxPreviewFps = settings.value("preview/maxFps", appSettings.maxPreviewFps).toInt();
    appSettings.previewScale = settings.value("preview/scale", appSettings.previewScale).toDouble();
    appSettings.allowFrameSkip = settings.value("preview/allowFrameSkip", appSettings.allowFrameSkip).toBool();
    appSettings.previewFrameStep = settings.value("preview/frameStep", appSettings.previewFrameStep).toInt();
    appSettings.offlineVideoStartFrame = settings.value("task/offlineVideoStartFrame", appSettings.offlineVideoStartFrame).toInt();
    appSettings.offlineVideoEndFrame = settings.value("task/offlineVideoEndFrame", appSettings.offlineVideoEndFrame).toInt();
    appSettings.dnnBackend = settings.value("dnn/backend", appSettings.dnnBackend).toString();
    appSettings.exportVideoSideBySide = settings.value("export/videoSideBySide", appSettings.exportVideoSideBySide).toBool();
    appSettings.showWelcomeGuide = settings.value("ui/showWelcomeGuide", appSettings.showWelcomeGuide).toBool();
    return appSettings;
}

void save(const AppSettings& appSettings)
{
    QSettings settings(kOrganizationName, kApplicationName);
    settings.setValue("ui/languageCode", appSettings.languageCode);
    settings.setValue("export/defaultDirectory", appSettings.defaultExportDirectory);
    settings.setValue("export/autoOpenDirectory", appSettings.autoOpenExportDirectory);
    settings.setValue("media/recentFiles", appSettings.recentMediaFiles);
    settings.setValue("project/recentFiles", appSettings.recentProjectFiles);
    settings.setValue("media/maxRecentFiles", appSettings.maxRecentMediaFiles);
    settings.setValue("preview/maxFps", appSettings.maxPreviewFps);
    settings.setValue("preview/scale", appSettings.previewScale);
    settings.setValue("preview/allowFrameSkip", appSettings.allowFrameSkip);
    settings.setValue("preview/frameStep", appSettings.previewFrameStep);
    settings.setValue("task/offlineVideoStartFrame", appSettings.offlineVideoStartFrame);
    settings.setValue("task/offlineVideoEndFrame", appSettings.offlineVideoEndFrame);
    settings.setValue("dnn/backend", appSettings.dnnBackend);
    settings.setValue("export/videoSideBySide", appSettings.exportVideoSideBySide);
    settings.setValue("ui/showWelcomeGuide", appSettings.showWelcomeGuide);
}

}
