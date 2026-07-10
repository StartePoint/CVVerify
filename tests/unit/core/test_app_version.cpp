#include <QtTest>

#include "app/AppVersion.h"

class AppVersionTest : public QObject
{
    Q_OBJECT

private slots:
    void exposesApplicationVersion();
};

void AppVersionTest::exposesApplicationVersion()
{
    QCOMPARE(AppVersion::applicationName(), QStringLiteral("CVVerify"));
    QCOMPARE(AppVersion::applicationVersion(), QStringLiteral("0.1.0"));
    QCOMPARE(AppVersion::displayName(), QStringLiteral("CVVerify 0.1.0"));
}

int runAppVersionTests(int argc, char* argv[])
{
    AppVersionTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_app_version.moc"
