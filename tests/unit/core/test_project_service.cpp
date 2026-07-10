#include <QtTest>

#include <QTemporaryDir>

#include "core/project/ProjectService.h"

class ProjectServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void savesAndLoadsProjectDefinition();
};

void ProjectServiceTest::savesAndLoadsProjectDefinition()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ProjectDefinition project;
    project.projectId = "demo";
    project.displayName = "Demo Project";
    project.mediaSourcePath = "C:/images/sample.png";
    project.mediaSourceKind = "ImageFile";
    project.pipeline.displayName = "Test Pipeline";
    project.pipeline.steps.append({"builtin.grayscale", "Grayscale", {}});

    const QString filePath = dir.filePath("project.cvverify.json");
    QString errorMessage;
    QVERIFY(ProjectService::saveToFile(project, filePath, &errorMessage));

    const ProjectLoadResult result = ProjectService::loadFromFile(filePath);
    QVERIFY(result.success);
    QCOMPARE(result.project.projectId, QString("demo"));
    QCOMPARE(result.project.pipeline.displayName, QString("Test Pipeline"));
    QCOMPARE(result.project.pipeline.steps.size(), 1);
}

int runProjectServiceTests(int argc, char* argv[])
{
    ProjectServiceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_project_service.moc"
