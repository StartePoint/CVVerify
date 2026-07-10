#include <QtTest>

#include <QTemporaryDir>
#include <QFileInfo>

#include "core/pipeline/PipelineDefinition.h"
#include "core/pipeline/PipelineJsonSerializer.h"
#include "infra/platform/PlatformPaths.h"

class PipelineSerializationTest : public QObject
{
    Q_OBJECT

private slots:
    void savesAndLoadsPipelineDefinition();
    void loadsBundledSamplePipelines();
};

void PipelineSerializationTest::savesAndLoadsPipelineDefinition()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    PipelineDefinition definition;
    definition.pipelineId = "demo.pipeline";
    definition.displayName = "Demo Pipeline";
    definition.steps = {
        {"builtin.grayscale", "Gray Stage", {}},
        {"builtin.resize", "Resize Stage", {{"width", 640}, {"height", 480}}},
    };

    const QString filePath = dir.filePath("pipeline.json");
    QString errorMessage;

    QVERIFY(PipelineJsonSerializer::saveToFile(definition, filePath, &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    const PipelineLoadResult result = PipelineJsonSerializer::loadFromFile(filePath);

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.definition.pipelineId, QString("demo.pipeline"));
    QCOMPARE(result.definition.displayName, QString("Demo Pipeline"));
    QCOMPARE(result.definition.steps.size(), 2);
    QCOMPARE(result.definition.steps.at(0).displayName, QString("Gray Stage"));
    QCOMPARE(result.definition.steps.at(1).parameters.value("width").toInt(), 640);
}

void PipelineSerializationTest::loadsBundledSamplePipelines()
{
    const QStringList sampleNames = {
        "basic_blur",
        "edge_canny",
        "morphology_open",
        "histogram_sidecar",
    };

    for (const QString& sampleName : sampleNames) {
        const QString filePath = PlatformPaths::resolveResourcePath(
            QString("samples/pipelines/%1.json").arg(sampleName));
        QVERIFY2(QFileInfo::exists(filePath), qPrintable(filePath));

        const PipelineLoadResult result = PipelineJsonSerializer::loadFromFile(filePath);
        QVERIFY2(result.success, qPrintable(QString("%1: %2").arg(sampleName, result.errorMessage)));
        QVERIFY2(!result.definition.steps.isEmpty(), qPrintable(sampleName));
    }
}

int runPipelineSerializationTests(int argc, char* argv[])
{
    PipelineSerializationTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_pipeline_serialization.moc"
