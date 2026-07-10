#include "ui/dialogs/YoloModelImportDialog.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>

#include "core/detection/ModelTemplateLoader.h"
#include "ui_YoloModelImportDialog.h"

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

ModelSourceType sourceTypeFromComboData(const QVariant& value)
{
    return static_cast<ModelSourceType>(value.toInt());
}

}

YoloModelImportDialog::YoloModelImportDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::YoloModelImportDialog)
{
    ui->setupUi(this);
    populateSourceTypes();
    populateTemplates();

    connect(ui->browseModelButton, &QPushButton::clicked, this, &YoloModelImportDialog::handleBrowseModel);
    connect(ui->browseLabelsButton, &QPushButton::clicked, this, &YoloModelImportDialog::handleBrowseLabels);
    connect(ui->templateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &YoloModelImportDialog::handleTemplateChanged);
}

YoloModelImportDialog::~YoloModelImportDialog()
{
    delete ui;
}

void YoloModelImportDialog::populateSourceTypes()
{
    ui->sourceTypeComboBox->clear();
    ui->sourceTypeComboBox->addItem("YOLO Detection", static_cast<int>(ModelSourceType::YoloDetection));
    ui->sourceTypeComboBox->addItem("Generic Detection", static_cast<int>(ModelSourceType::GenericDetection));
    ui->sourceTypeComboBox->addItem("Classification", static_cast<int>(ModelSourceType::Classification));
    ui->sourceTypeComboBox->addItem("Segmentation", static_cast<int>(ModelSourceType::GenericSegmentation));
    ui->sourceTypeComboBox->addItem("OCR", static_cast<int>(ModelSourceType::Ocr));
    ui->sourceTypeComboBox->addItem("Keypoint", static_cast<int>(ModelSourceType::Keypoint));
    ui->sourceTypeComboBox->addItem("Custom Tensor", static_cast<int>(ModelSourceType::Custom));
}

void YoloModelImportDialog::populateTemplates()
{
    ui->templateComboBox->clear();
    ui->templateComboBox->addItem("(None)", QString());
    for (const QString& templateName : ModelTemplateLoader::listTemplates()) {
        ui->templateComboBox->addItem(templateName, templateName);
    }
}

void YoloModelImportDialog::applyTemplateByName(const QString& templateName)
{
    if (templateName.trimmed().isEmpty()) {
        return;
    }

    DetectionModelDescriptor templateDescriptor;
    QString errorMessage;
    if (!ModelTemplateLoader::loadTemplate(templateName, &templateDescriptor, &errorMessage)) {
        return;
    }

    const int sourceTypeIndex = ui->sourceTypeComboBox->findData(
        static_cast<int>(ModelTemplateLoader::sourceTypeFromTaskType(templateDescriptor.taskType)));
    if (sourceTypeIndex >= 0) {
        ui->sourceTypeComboBox->setCurrentIndex(sourceTypeIndex);
    }

    if (ui->modelNameEdit->text().trimmed().isEmpty() && !templateDescriptor.modelName.isEmpty()) {
        ui->modelNameEdit->setText(templateDescriptor.modelName);
    }
}

void YoloModelImportDialog::handleBrowseModel()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        windowTitle(),
        ui->modelPathEdit->text(),
        "ONNX Models (*.onnx)");
    if (!filePath.isEmpty()) {
        ui->modelPathEdit->setText(filePath);
        if (ui->modelNameEdit->text().trimmed().isEmpty()) {
            ui->modelNameEdit->setText(QFileInfo(filePath).completeBaseName());
        }
    }
}

void YoloModelImportDialog::handleBrowseLabels()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        windowTitle(),
        ui->labelsPathEdit->text(),
        "Label Files (*.txt *.names)");
    if (!filePath.isEmpty()) {
        ui->labelsPathEdit->setText(filePath);
    }
}

void YoloModelImportDialog::handleTemplateChanged(int index)
{
    if (index < 0) {
        return;
    }

    applyTemplateByName(ui->templateComboBox->itemData(index).toString());
}

YoloImportRequest YoloModelImportDialog::buildRequest() const
{
    const ModelImportRequest request = buildImportRequest();
    YoloImportRequest yoloRequest;
    yoloRequest.modelPath = request.modelPath;
    yoloRequest.labelsPath = request.labelsPath;
    yoloRequest.modelName = request.modelName;
    return yoloRequest;
}

ModelImportRequest YoloModelImportDialog::buildImportRequest() const
{
    ModelImportRequest request;
    request.modelPath = ui->modelPathEdit->text().trimmed();
    request.labelsPath = ui->labelsPathEdit->text().trimmed();
    request.modelName = ui->modelNameEdit->text().trimmed();
    request.sourceType = sourceTypeFromComboData(ui->sourceTypeComboBox->currentData());
    return request;
}

QString YoloModelImportDialog::selectedTemplateName() const
{
    return ui->templateComboBox->currentData().toString();
}

void YoloModelImportDialog::setInitialModelPath(const QString& modelPath)
{
    const QString trimmedPath = modelPath.trimmed();
    if (trimmedPath.isEmpty()) {
        return;
    }

    ui->modelPathEdit->setText(trimmedPath);
    if (ui->modelNameEdit->text().trimmed().isEmpty()) {
        ui->modelNameEdit->setText(QFileInfo(trimmedPath).completeBaseName());
    }

    const QString siblingLabelsPath = QFileInfo(trimmedPath).absolutePath() + QStringLiteral("/labels.txt");
    if (ui->labelsPathEdit->text().trimmed().isEmpty() && QFileInfo::exists(siblingLabelsPath)) {
        ui->labelsPathEdit->setText(siblingLabelsPath);
    }
}

void YoloModelImportDialog::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = isChineseLanguage(languageCode);

    setWindowTitle(chinese ? QStringLiteral("导入 ONNX 模型") : "Import ONNX Model");
    ui->sourceTypeLabel->setText(chinese ? QStringLiteral("模型类别") : "Model Category");
    ui->templateLabel->setText(chinese ? QStringLiteral("配置模板") : "Template");
    ui->modelPathLabel->setText(chinese ? QStringLiteral("模型路径") : "Model Path");
    ui->labelsPathLabel->setText(chinese ? QStringLiteral("标签文件") : "Labels Path");
    ui->modelNameLabel->setText(chinese ? QStringLiteral("模型名称") : "Model Name");
    ui->browseModelButton->setText(chinese ? QStringLiteral("浏览...") : "Browse...");
    ui->browseLabelsButton->setText(chinese ? QStringLiteral("浏览...") : "Browse...");

    if (ui->templateComboBox->count() > 0 && ui->templateComboBox->itemData(0).toString().isEmpty()) {
        ui->templateComboBox->setItemText(0, chinese ? QStringLiteral("(无)") : "(None)");
    }

    const int yoloIndex = ui->sourceTypeComboBox->findData(static_cast<int>(ModelSourceType::YoloDetection));
    if (yoloIndex >= 0) {
        ui->sourceTypeComboBox->setItemText(yoloIndex, chinese ? QStringLiteral("YOLO 检测") : "YOLO Detection");
    }
    const int detectionIndex = ui->sourceTypeComboBox->findData(static_cast<int>(ModelSourceType::GenericDetection));
    if (detectionIndex >= 0) {
        ui->sourceTypeComboBox->setItemText(detectionIndex, chinese ? QStringLiteral("通用检测") : "Generic Detection");
    }
    const int classificationIndex = ui->sourceTypeComboBox->findData(static_cast<int>(ModelSourceType::Classification));
    if (classificationIndex >= 0) {
        ui->sourceTypeComboBox->setItemText(classificationIndex, chinese ? QStringLiteral("图像分类") : "Classification");
    }
    const int segmentationIndex = ui->sourceTypeComboBox->findData(static_cast<int>(ModelSourceType::GenericSegmentation));
    if (segmentationIndex >= 0) {
        ui->sourceTypeComboBox->setItemText(segmentationIndex, chinese ? QStringLiteral("语义分割") : "Segmentation");
    }
    const int customIndex = ui->sourceTypeComboBox->findData(static_cast<int>(ModelSourceType::Custom));
    if (customIndex >= 0) {
        ui->sourceTypeComboBox->setItemText(customIndex, chinese ? QStringLiteral("自定义 Tensor") : "Custom Tensor");
    }
}
