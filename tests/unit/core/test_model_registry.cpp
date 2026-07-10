#include <QtTest>

#include "core/models/ModelRegistry.h"

class ModelRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void registersAndFindsPackages();
};

void ModelRegistryTest::registersAndFindsPackages()
{
    ModelRegistry registry;

    RegisteredModelPackage package;
    package.packageDir = "models/demo";
    package.modelPath = "models/demo/model.onnx";
    package.descriptor.modelName = "demo";
    package.labels = QStringList() << "person" << "car";

    QVERIFY(registry.registerPackage(package));
    QCOMPARE(registry.activeModelName(), QString("demo"));

    RegisteredModelPackage loaded;
    QVERIFY(registry.findByName("demo", &loaded));
    QCOMPARE(loaded.labels.size(), 2);
}

int runModelRegistryTests(int argc, char* argv[])
{
    ModelRegistryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_model_registry.moc"
