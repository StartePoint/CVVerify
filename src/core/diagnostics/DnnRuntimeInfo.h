#pragma once

#include <QString>
#include <QStringList>

namespace DnnRuntimeInfo {

struct BackendInfo
{
    QString preferredBackend = "default";
    QString preferredTarget = "cpu";
    QString opencvVersion;
    QStringList availableBackends;
};

BackendInfo queryBackendInfo();
void logBackendInfo();

}
