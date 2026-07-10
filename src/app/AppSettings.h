#pragma once

#include <QString>
#include <QStringList>

struct AppSettings
{
    QString languageCode = "en";
    QString defaultExportDirectory;
    bool autoOpenExportDirectory = false;
    QStringList recentMediaFiles;
    QStringList recentProjectFiles;
    int maxRecentMediaFiles = 10;
    int maxPreviewFps = 30;
    double previewScale = 1.0;
    bool allowFrameSkip = true;
    int previewFrameStep = 1;
    int offlineVideoStartFrame = 0;
    int offlineVideoEndFrame = -1;
    QString dnnBackend = "cpu";
    bool exportVideoSideBySide = false;
    bool showWelcomeGuide = true;
};

namespace AppSettingsStore {

AppSettings load();
void save(const AppSettings& settings);

}
