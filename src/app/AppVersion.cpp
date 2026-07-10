#include "app/AppVersion.h"

namespace AppVersion {

QString applicationVersion()
{
    return QStringLiteral("0.1.0");
}

QString applicationName()
{
    return QStringLiteral("CVVerify");
}

QString displayName()
{
    return QStringLiteral("%1 %2").arg(applicationName(), applicationVersion());
}

}
