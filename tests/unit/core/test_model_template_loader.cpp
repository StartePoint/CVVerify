#include <QtTest>

#include "core/detection/ModelTemplateLoader.h"

class ModelTemplateLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void mapsTaskTypeToSourceType();
    void loadsBuiltInClassificationTemplate();
    void applyTemplatePreservesModelName();
    void loadsBuiltInSegmentationTemplate();
};

void ModelTemplateLoaderTest::mapsTaskTypeToSourceType()
{
    QCOMPARE(ModelTemplateLoader::sourceTypeFromTaskType("classification"), ModelSourceType::Classification);
    QCOMPARE(ModelTemplateLoader::sourceTypeFromTaskType("segmentation"), ModelSourceType::GenericSegmentation);
    QCOMPARE(ModelTemplateLoader::sourceTypeFromTaskType("detection"), ModelSourceType::GenericDetection);
    QCOMPARE(ModelTemplateLoader::sourceTypeFromTaskType("ocr"), ModelSourceType::Ocr);
    QCOMPARE(ModelTemplateLoader::sourceTypeFromTaskType("keypoint"), ModelSourceType::Keypoint);
}

void ModelTemplateLoaderTest::loadsBuiltInClassificationTemplate()
{
    QVERIFY(ModelTemplateLoader::listTemplates().contains("classification"));

    DetectionModelDescriptor descriptor;
    QString errorMessage;
    QVERIFY(ModelTemplateLoader::loadTemplate("classification", &descriptor, &errorMessage));
    QCOMPARE(descriptor.taskType, QString("classification"));
    QCOMPARE(descriptor.input.width, 224);
    QCOMPARE(descriptor.output.layoutType, QString("classification_logits"));
}

void ModelTemplateLoaderTest::applyTemplatePreservesModelName()
{
    DetectionModelDescriptor target;
    target.modelName = "runtime_model";
    target.taskType = "detection";

    DetectionModelDescriptor templateDescriptor;
    templateDescriptor.modelName = "template_name";
    templateDescriptor.taskType = "classification";
    templateDescriptor.input.width = 299;

    ModelTemplateLoader::applyTemplateDefaults(target, templateDescriptor);
    QCOMPARE(target.modelName, QString("runtime_model"));
    QCOMPARE(target.taskType, QString("classification"));
    QCOMPARE(target.input.width, 299);
}

void ModelTemplateLoaderTest::loadsBuiltInSegmentationTemplate()
{
    QVERIFY(ModelTemplateLoader::listTemplates().contains("segmentation"));

    DetectionModelDescriptor descriptor;
    QString errorMessage;
    QVERIFY(ModelTemplateLoader::loadTemplate("segmentation", &descriptor, &errorMessage));
    QCOMPARE(descriptor.taskType, QString("segmentation"));
    QCOMPARE(descriptor.input.width, 512);
    QCOMPARE(descriptor.output.layoutType, QString("segmentation_mask"));
}

int runModelTemplateLoaderTests(int argc, char* argv[])
{
    ModelTemplateLoaderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_model_template_loader.moc"
