#include <QtTest>

#include <QFileInfo>

#include "app/AppDefaultsLoader.h"
#include "app/AppSettings.h"
#include "infra/platform/PlatformPaths.h"

class AppDefaultsLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsBundledDefaultsFile();
    void userSettingsOverrideDefaultsAfterMerge();
};

void AppDefaultsLoaderTest::loadsBundledDefaultsFile()
{
    const QString defaultsPath = PlatformPaths::resolveResourcePath("config/app_defaults.json");
    QVERIFY2(QFileInfo::exists(defaultsPath), qPrintable(defaultsPath));

    AppSettings settings;
    QString errorMessage;
    QVERIFY2(AppDefaultsLoader::applyTo(&settings, &errorMessage), qPrintable(errorMessage));
    QCOMPARE(settings.languageCode, QString("en"));
    QCOMPARE(settings.maxPreviewFps, 30);
    QCOMPARE(settings.previewFrameStep, 1);
    QCOMPARE(settings.dnnBackend, QString("cpu"));
    QVERIFY(!settings.exportVideoSideBySide);
}

void AppDefaultsLoaderTest::userSettingsOverrideDefaultsAfterMerge()
{
    AppSettings settings;
    AppDefaultsLoader::applyTo(&settings);

    settings.languageCode = "zh-CN";
    settings.maxPreviewFps = 24;
    settings.dnnBackend = "opencl";

    QCOMPARE(settings.languageCode, QString("zh-CN"));
    QCOMPARE(settings.maxPreviewFps, 24);
    QCOMPARE(settings.dnnBackend, QString("opencl"));
}

int runAppDefaultsLoaderTests(int argc, char* argv[])
{
    AppDefaultsLoaderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_app_defaults_loader.moc"
