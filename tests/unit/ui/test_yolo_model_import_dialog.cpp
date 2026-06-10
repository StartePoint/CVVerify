#include <QtTest>

#include <QLineEdit>

#include "ui/dialogs/YoloModelImportDialog.h"

class YoloModelImportDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void buildsImportRequestFromFields();
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

int runYoloModelImportDialogTests(int argc, char* argv[])
{
    YoloModelImportDialogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_model_import_dialog.moc"
