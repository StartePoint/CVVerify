#pragma once

#include <QString>

struct AppSettings
{
    QString languageCode = "en";
    QString defaultExportDirectory;
    bool autoOpenExportDirectory = false;
};

namespace AppSettingsStore {

AppSettings load();
void save(const AppSettings& settings);

}
