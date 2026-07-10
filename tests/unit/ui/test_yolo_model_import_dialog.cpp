#include <QtTest>

#include <QComboBox>
#include <QLineEdit>

#include "core/detection/ModelImportService.h"
#include "ui/dialogs/YoloModelImportDialog.h"

class YoloModelImportDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void buildsImportRequestFromFields();
    void buildsTypedImportRequest();
};

void YoloModelImportDialogTest::buildsImportRequestFromFields()
{
    YoloModelImportDialog dialog;

    dialog.findChild<QLineEdit*>("modelPathEdit")->setText("model.onnx");
    dialog.findChild<QLineEdit*>("labelsPathEdit")->setText("labels.txt");
    dialog.findChild<QLineEdit*>("modelNameEdit")->setText("yolo_demo");

    const YoloImportRequest request = dialog.buildRequest();

    QCOMPARE(request.modelPath, QString("model.onnx"));
    QCOMPARE(request.labelsPath, QString("labels.txt"));
    QCOMPARE(request.modelName, QString("yolo_demo"));
}

void YoloModelImportDialogTest::buildsTypedImportRequest()
{
    YoloModelImportDialog dialog;

    auto* sourceTypeCombo = dialog.findChild<QComboBox*>("sourceTypeComboBox");
    QVERIFY(sourceTypeCombo != nullptr);
    const int classificationIndex = sourceTypeCombo->findData(static_cast<int>(ModelSourceType::Classification));
    QVERIFY(classificationIndex >= 0);
    sourceTypeCombo->setCurrentIndex(classificationIndex);

    dialog.findChild<QLineEdit*>("modelPathEdit")->setText("cls.onnx");
    dialog.findChild<QLineEdit*>("labelsPathEdit")->setText("cls.txt");
    dialog.findChild<QLineEdit*>("modelNameEdit")->setText("cls_demo");

    const ModelImportRequest request = dialog.buildImportRequest();
    QCOMPARE(request.sourceType, ModelSourceType::Classification);
    QCOMPARE(request.modelPath, QString("cls.onnx"));
    QCOMPARE(request.modelName, QString("cls_demo"));
}

int runYoloModelImportDialogTests(int argc, char* argv[])
{
    YoloModelImportDialogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_model_import_dialog.moc"
