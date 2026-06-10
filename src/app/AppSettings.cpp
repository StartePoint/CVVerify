#include "app/AppSettings.h"

#include <QSettings>

namespace {

const char* const kOrganizationName = "StartePoint";
const char* const kApplicationName = "CVVerify";

}

namespace AppSettingsStore {

AppSettings load()
{
    QSettings settings(kOrganizationName, kApplicationName);

    AppSettings appSettings;
    appSettings.languageCode = settings.value("ui/languageCode", appSettings.languageCode).toString();
    appSettings.defaultExportDirectory = settings.value("export/defaultDirectory", QString()).toString();
    appSettings.autoOpenExportDirectory = settings.value("export/autoOpenDirectory", false).toBool();
    return appSettings;
}

void save(const AppSettings& appSettings)
{
    QSettings settings(kOrganizationName, kApplicationName);
    settings.setValue("ui/languageCode", appSettings.languageCode);
    settings.setValue("export/defaultDirectory", appSettings.defaultExportDirectory);
    settings.setValue("export/autoOpenDirectory", appSettings.autoOpenExportDirectory);
}

}
