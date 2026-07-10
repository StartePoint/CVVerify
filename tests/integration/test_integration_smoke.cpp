#include <QtTest>

#include <QDir>
#include <QFileInfo>

#include "app/ServiceLocator.h"
#include "core/diagnostics/DiagnosticsLog.h"
#include "core/models/ModelRegistry.h"
#include "core/operators/OperatorRegistry.h"
#include "core/tasks/TaskScheduler.h"
#include "core/project/ProjectService.h"
#include "core/operators/OperatorSchemaLoader.h"
#include "infra/platform/PlatformPaths.h"

class IntegrationSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void serviceLocatorExposesCoreServices();
    void platformPathsCreateWritableDirectories();
    void loadsBundledDemoProjectSample();
    void resolvesBundledConfigAndSamples();
    void resolvesDocumentationFromRepository();
};

void IntegrationSmokeTest::serviceLocatorExposesCoreServices()
{
    QVERIFY(ServiceLocator::instance().operatorRegistry().allOperators().size() > 0);
    QVERIFY(ServiceLocator::instance().modelRegistry().allPackages().size() >= 0);
    QVERIFY(!ServiceLocator::instance().taskScheduler().isRunning());
    DiagnosticsLog::instance().info("integration", "smoke test");
    QVERIFY(!DiagnosticsLog::instance().entries().isEmpty());
}

void IntegrationSmokeTest::platformPathsCreateWritableDirectories()
{
    QVERIFY(QDir(PlatformPaths::configDirectory()).exists());
    QVERIFY(QDir(PlatformPaths::modelsDirectory()).exists());
    QVERIFY(QDir(PlatformPaths::exportsDirectory()).exists());
    QVERIFY(QDir(PlatformPaths::presetsDirectory()).exists());
}

void IntegrationSmokeTest::loadsBundledDemoProjectSample()
{
    const QString projectPath = PlatformPaths::resolveResourcePath("samples/projects/demo_project.json");
    QVERIFY2(QFileInfo::exists(projectPath), qPrintable(projectPath));

    const ProjectLoadResult result = ProjectService::loadFromFile(projectPath);
    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.project.projectId, QString("samples.demo"));
    QCOMPARE(result.project.pipeline.steps.size(), 1);
    QCOMPARE(result.project.pipeline.steps.first().stepId, QString("builtin.gaussian_blur"));
}

void IntegrationSmokeTest::resolvesBundledConfigAndSamples()
{
    const QString schemaDir = PlatformPaths::resolveResourcePath("config/operator_schemas");
    QVERIFY(QDir(schemaDir).exists());

    const QString sampleImage = PlatformPaths::resolveResourcePath("samples/images/checkerboard.png");
    QVERIFY2(QFileInfo::exists(sampleImage), qPrintable(sampleImage));

    QStringList failures;
    const int failureCount = OperatorSchemaLoader::validateIndexedSchemas(
        ServiceLocator::instance().operatorRegistry(),
        &failures);
    if (failureCount > 0) {
        qWarning() << "Schema validation failures:" << failures;
    }
    QCOMPARE(failureCount, 0);
}

void IntegrationSmokeTest::resolvesDocumentationFromRepository()
{
    const QString quickStartPath = PlatformPaths::resolveDocumentationPath("quick-start-validation.md");
    QVERIFY2(QFileInfo::exists(quickStartPath), qPrintable(quickStartPath));

    const QString manualValidationPath = PlatformPaths::resolveDocumentationPath("manual-validation-v0.1.0.md");
    QVERIFY2(QFileInfo::exists(manualValidationPath), qPrintable(manualValidationPath));
}

int runIntegrationSmokeTests(int argc, char* argv[])
{
    IntegrationSmokeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_integration_smoke.moc"
