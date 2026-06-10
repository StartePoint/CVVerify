#pragma once

#include <QDialog>

#include "core/detection/YoloOnnxImportService.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class YoloModelImportDialog;
}
QT_END_NAMESPACE

class YoloModelImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit YoloModelImportDialog(QWidget* parent = nullptr);
    ~YoloModelImportDialog() override;

    YoloImportRequest buildRequest() const;

private:
    Ui::YoloModelImportDialog* ui = nullptr;
};
