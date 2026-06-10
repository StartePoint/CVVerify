#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/YoloDetectionStep.h"

class YoloDetectionStepTest : public QObject
{
    Q_OBJECT

private slots:
    void reportsMissingModelAsFailure();
};

void YoloDetectionStepTest::reportsMissingModelAsFailure()
{
    DetectionModelDescriptor descriptor;
    descriptor.modelName = "missing";

    YoloDetectionStep step(descriptor, "missing-model.onnx", QStringList() << "person");

    FramePacket frame;
    frame.originalMat = cv::Mat(16, 16, CV_8UC3, cv::Scalar(0, 0, 0));
    frame.workingMat = frame.originalMat.clone();

    const StepResult result = step.execute(frame, {});

    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
}

int runYoloDetectionStepTests(int argc, char* argv[])
{
    YoloDetectionStepTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_detection_step.moc"
