#include <QtTest>

#include <QTemporaryDir>

#include "core/detection/ModelPackageLoader.h"

class ModelPackageLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void savesAndLoadsDetectionModelDescriptor();
};

void ModelPackageLoaderTest::savesAndLoadsDetectionModelDescriptor()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DetectionModelDescriptor descriptor;
    descriptor.modelName = "yolo_demo";
    descriptor.taskType = "detection";
    descriptor.frameworkFamily = "yolo";
    descriptor.modelFormat = "onnx";
    descriptor.labelsPath = "labels.txt";
    descriptor.input.width = 640;
    descriptor.input.height = 640;
    descriptor.postprocess.confidenceThreshold = 0.25f;
    descriptor.postprocess.nmsIouThreshold = 0.45f;

    QString errorMessage;
    QVERIFY(ModelPackageLoader::saveDescriptor(descriptor, dir.path(), &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    const DetectionModelLoadResult result = ModelPackageLoader::loadDescriptor(dir.path());

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.descriptor.modelName, QString("yolo_demo"));
    QCOMPARE(result.descriptor.input.width, 640);
    QCOMPARE(result.descriptor.input.height, 640);
    QCOMPARE(result.descriptor.postprocess.confidenceThreshold, 0.25f);
    QCOMPARE(result.descriptor.postprocess.nmsIouThreshold, 0.45f);
}

int runModelPackageLoaderTests(int argc, char* argv[])
{
    ModelPackageLoaderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_model_package_loader.moc"
