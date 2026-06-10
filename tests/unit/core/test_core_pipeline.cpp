#include <QtTest>

#include <memory>

#include "core/frame/FramePacket.h"
#include "core/pipeline/PipelineEngine.h"

class FakeStep : public IPipelineStep
{
public:
    explicit FakeStep(bool success)
        : m_success(success)
    {
    }

    QString id() const override
    {
        return "fake";
    }

    QString displayName() const override
    {
        return "Fake";
    }

    StepSchema schema() const override
    {
        return {};
    }

    QVariantMap parameterValues() const override
    {
        return {};
    }

    void setParameterValues(const QVariantMap&) override
    {
    }

    StepResult execute(FramePacket&, const RunContext&) override
    {
        if (m_success) {
            return {};
        }

        return {false, "step failed"};
    }

private:
    bool m_success = true;
};

class CorePipelineTest : public QObject
{
    Q_OBJECT

private slots:
    void framePacketDefaultsAreInitialized();
    void pipelineReturnsSuccessWhenAllStepsPass();
    void pipelineStopsWhenAStepFails();
};

void CorePipelineTest::framePacketDefaultsAreInitialized()
{
    FramePacket frame;

    QCOMPARE(frame.frameId, -1);
    QCOMPARE(frame.timestampMs, 0);
    QVERIFY(frame.sourceId.isEmpty());
    QVERIFY(frame.originalMat.empty());
    QVERIFY(frame.workingMat.empty());
    QVERIFY(frame.artifacts.isEmpty());
    QVERIFY(frame.tensorOutputs.isEmpty());
}

void CorePipelineTest::pipelineReturnsSuccessWhenAllStepsPass()
{
    PipelineEngine engine;
    FramePacket frame;

    engine.setSteps({std::make_shared<FakeStep>(true), std::make_shared<FakeStep>(true)});

    StepResult result = engine.run(frame, RunContext{});

    QVERIFY(result.success);
    QVERIFY(result.errorMessage.isEmpty());
}

void CorePipelineTest::pipelineStopsWhenAStepFails()
{
    PipelineEngine engine;
    FramePacket frame;

    engine.setSteps({std::make_shared<FakeStep>(true), std::make_shared<FakeStep>(false)});

    StepResult result = engine.run(frame, RunContext{});

    QVERIFY(!result.success);
    QCOMPARE(result.errorMessage, QString("step failed"));
}

int runCorePipelineTests(int argc, char* argv[])
{
    CorePipelineTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_core_pipeline.moc"
