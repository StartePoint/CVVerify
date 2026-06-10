#include <QtTest>

#include <QTemporaryDir>

#include "core/pipeline/PipelineDefinition.h"
#include "core/pipeline/PipelineJsonSerializer.h"

class PipelineSerializationTest : public QObject
{
    Q_OBJECT

private slots:
    void savesAndLoadsPipelineDefinition();
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

int runPipelineSerializationTests(int argc, char* argv[])
{
    PipelineSerializationTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_pipeline_serialization.moc"
