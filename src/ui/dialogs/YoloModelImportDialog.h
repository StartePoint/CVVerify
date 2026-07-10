#pragma once

#include <QDialog>

#include "core/detection/ModelImportService.h"
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
    ModelImportRequest buildImportRequest() const;
    QString selectedTemplateName() const;

    void setLanguage(const QString& languageCode);
    void setInitialModelPath(const QString& modelPath);

private slots:
    void handleBrowseModel();
    void handleBrowseLabels();
    void handleTemplateChanged(int index);

private:
    void populateSourceTypes();
    void populateTemplates();
    void applyTemplateByName(const QString& templateName);

    Ui::YoloModelImportDialog* ui = nullptr;
    QString m_languageCode = "en";
};
