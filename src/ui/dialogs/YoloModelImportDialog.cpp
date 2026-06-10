#include "ui/dialogs/YoloModelImportDialog.h"

#include "ui_YoloModelImportDialog.h"

YoloModelImportDialog::YoloModelImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::YoloModelImportDialog)
{
    ui->setupUi(this);
}

YoloModelImportDialog::~YoloModelImportDialog()
{
    delete ui;
}

YoloImportRequest YoloModelImportDialog::buildRequest() const
{
    YoloImportRequest request;
    request.modelPath = ui->modelPathEdit->text().trimmed();
    request.labelsPath = ui->labelsPathEdit->text().trimmed();
    request.modelName = ui->modelNameEdit->text().trimmed();
    return request;
}
