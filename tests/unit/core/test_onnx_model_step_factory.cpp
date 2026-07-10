#include <QtTest>

#include "core/detection/OnnxModelStepFactory.h"

class OnnxModelStepFactoryTest : public QObject
{
    Q_OBJECT

private slots:
    void createsDetectionStepByDefault();
    void createsClassificationStep();
    void createsSegmentationStep();
    void createsCustomTensorStep();
    void createsOcrStep();
    void createsKeypointStep();
};

void OnnxModelStepFactoryTest::createsDetectionStepByDefault()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "detection";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList());
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("detection.yolo"));
}

void OnnxModelStepFactoryTest::createsClassificationStep()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "classification";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList() << "cat");
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("dnn.classification"));
}

void OnnxModelStepFactoryTest::createsSegmentationStep()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "segmentation";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList());
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("dnn.segmentation"));
}

void OnnxModelStepFactoryTest::createsCustomTensorStep()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "custom";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList());
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("dnn.custom_tensor"));
}

void OnnxModelStepFactoryTest::createsOcrStep()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "ocr";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList());
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("dnn.ocr"));
}

void OnnxModelStepFactoryTest::createsKeypointStep()
{
    DetectionModelDescriptor descriptor;
    descriptor.taskType = "keypoint";

    const auto step = OnnxModelStepFactory::createStep(descriptor, "model.onnx", QStringList());
    QVERIFY(step != nullptr);
    QCOMPARE(step->id(), QString("dnn.keypoint"));
}

int runOnnxModelStepFactoryTests(int argc, char* argv[])
{
    OnnxModelStepFactoryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_onnx_model_step_factory.moc"
