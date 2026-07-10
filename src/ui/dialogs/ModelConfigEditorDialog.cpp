#include "ui/dialogs/ModelConfigEditorDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <QJsonDocument>
#include <QJsonObject>

#include "core/detection/DetectionRenderComposer.h"
#include "core/detection/DetectionTypes.h"
#include "core/detection/ModelTemplateLoader.h"
#include "core/detection/OnnxModelStepFactory.h"
#include "core/frame/FramePacket.h"
#include "core/pipeline/IPipelineStep.h"
#include "infra/opencv/OpenCvQtImageConverter.h"
#include "infra/platform/PlatformPaths.h"
#include "ui/widgets/CanvasView.h"

ModelConfigEditorDialog::ModelConfigEditorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Model Configuration");
    resize(720, 560);

    m_previewDebounceTimer = new QTimer(this);
    m_previewDebounceTimer->setSingleShot(true);
    m_previewDebounceTimer->setInterval(400);
    connect(m_previewDebounceTimer, &QTimer::timeout, this, &ModelConfigEditorDialog::runPreviewInference);

    auto* layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);

    auto* basicTab = new QWidget(this);
    auto* basicForm = new QFormLayout(basicTab);
    m_modelNameEdit = new QLineEdit(basicTab);
    m_taskTypeEdit = new QLineEdit(basicTab);
    m_frameworkEdit = new QLineEdit(basicTab);
    m_labelsPathEdit = new QLineEdit(basicTab);
    basicForm->addRow("Model Name", m_modelNameEdit);
    basicForm->addRow("Task Type", m_taskTypeEdit);
    basicForm->addRow("Framework", m_frameworkEdit);
    basicForm->addRow("Labels Path", m_labelsPathEdit);
    m_tabs->addTab(basicTab, "Basic");

    auto* inputTab = new QWidget(this);
    auto* inputForm = new QFormLayout(inputTab);
    m_inputWidthSpin = new QSpinBox(inputTab);
    m_inputHeightSpin = new QSpinBox(inputTab);
    m_inputChannelsSpin = new QSpinBox(inputTab);
    m_colorOrderCombo = new QComboBox(inputTab);
    m_inputWidthSpin->setRange(32, 4096);
    m_inputHeightSpin->setRange(32, 4096);
    m_inputChannelsSpin->setRange(1, 4);
    m_colorOrderCombo->addItems({"RGB", "BGR"});
    inputForm->addRow("Width", m_inputWidthSpin);
    inputForm->addRow("Height", m_inputHeightSpin);
    inputForm->addRow("Channels", m_inputChannelsSpin);
    inputForm->addRow("Color Order", m_colorOrderCombo);
    m_tabs->addTab(inputTab, "Input");

    auto* preprocessTab = new QWidget(this);
    auto* preprocessForm = new QFormLayout(preprocessTab);
    m_letterboxCheck = new QCheckBox(preprocessTab);
    m_keepRatioCheck = new QCheckBox(preprocessTab);
    m_scaleSpin = new QDoubleSpinBox(preprocessTab);
    m_scaleSpin->setRange(0.0001, 1.0);
    m_scaleSpin->setDecimals(7);
    preprocessForm->addRow("Letterbox", m_letterboxCheck);
    preprocessForm->addRow("Keep Ratio", m_keepRatioCheck);
    preprocessForm->addRow("Scale", m_scaleSpin);
    m_tabs->addTab(preprocessTab, "Preprocess");

    auto* postTab = new QWidget(this);
    auto* postForm = new QFormLayout(postTab);
    m_confidenceSpin = new QDoubleSpinBox(postTab);
    m_nmsSpin = new QDoubleSpinBox(postTab);
    m_maxDetectionsSpin = new QSpinBox(postTab);
    m_confidenceSpin->setRange(0.0, 1.0);
    m_nmsSpin->setRange(0.0, 1.0);
    m_maxDetectionsSpin->setRange(1, 10000);
    postForm->addRow("Confidence", m_confidenceSpin);
    postForm->addRow("NMS IoU", m_nmsSpin);
    postForm->addRow("Max Detections", m_maxDetectionsSpin);
    m_tabs->addTab(postTab, "Postprocess");

    auto* outputTab = new QWidget(this);
    auto* outputForm = new QFormLayout(outputTab);
    m_layoutTypeEdit = new QLineEdit(outputTab);
    outputForm->addRow("Output Layout", m_layoutTypeEdit);
    outputForm->addRow("Tensor Mapping", new QLabel("Use layout type to select decoder behavior.", outputTab));
    m_tabs->addTab(outputTab, "Output");

    auto* visualizationTab = new QWidget(this);
    auto* visualizationLayout = new QVBoxLayout(visualizationTab);
    m_previewCanvas = new CanvasView(visualizationTab);
    m_previewCanvas->setMinimumHeight(220);
    m_previewStatusLabel = new QLabel("Load media in the main window, then run preview here.", visualizationTab);
    m_runPreviewButton = new QPushButton("Run Preview", visualizationTab);
    visualizationLayout->addWidget(m_previewCanvas, 1);
    visualizationLayout->addWidget(m_previewStatusLabel);
    visualizationLayout->addWidget(m_runPreviewButton);
    m_tabs->addTab(visualizationTab, "Live Preview");

    layout->addWidget(m_tabs);

    connectPreviewSignals();
    connect(m_runPreviewButton, &QPushButton::clicked, this, &ModelConfigEditorDialog::handleRunPreview);

    auto* templateRow = new QHBoxLayout();
    m_loadTemplateButton = new QPushButton("Load Template", this);
    m_saveTemplateButton = new QPushButton("Save As Template", this);
    templateRow->addWidget(m_loadTemplateButton);
    templateRow->addWidget(m_saveTemplateButton);
    templateRow->addStretch();
    layout->addLayout(templateRow);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_saveTemplateButton, &QPushButton::clicked, this, &ModelConfigEditorDialog::handleSaveTemplate);
    connect(m_loadTemplateButton, &QPushButton::clicked, this, &ModelConfigEditorDialog::handleLoadTemplate);
    layout->addWidget(buttons);
    rebuildTexts();
}

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedText(const QString& languageCode, const QString& english, const QString& chinese)
{
    return isChineseLanguage(languageCode) ? chinese : english;
}

void setFormLabel(QFormLayout* layout, QWidget* field, const QString& text)
{
    if (!layout || !field) {
        return;
    }
    if (QLabel* label = qobject_cast<QLabel*>(layout->labelForField(field))) {
        label->setText(text);
    }
}

}

void ModelConfigEditorDialog::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    rebuildTexts();
}

void ModelConfigEditorDialog::rebuildTexts()
{
    const QString languageCode = m_languageCode;
    setWindowTitle(localizedText(languageCode, "Model Configuration", QStringLiteral("模型配置")));

    if (m_tabs) {
        m_tabs->setTabText(0, localizedText(languageCode, "Basic", QStringLiteral("基础")));
        m_tabs->setTabText(1, localizedText(languageCode, "Input", QStringLiteral("输入")));
        m_tabs->setTabText(2, localizedText(languageCode, "Preprocess", QStringLiteral("预处理")));
        m_tabs->setTabText(3, localizedText(languageCode, "Postprocess", QStringLiteral("后处理")));
        m_tabs->setTabText(4, localizedText(languageCode, "Output", QStringLiteral("输出")));
        m_tabs->setTabText(5, localizedText(languageCode, "Live Preview", QStringLiteral("实时预览")));
    }

    if (auto* basicForm = qobject_cast<QFormLayout*>(m_tabs->widget(0)->layout())) {
        setFormLabel(basicForm, m_modelNameEdit, localizedText(languageCode, "Model Name", QStringLiteral("模型名称")));
        setFormLabel(basicForm, m_taskTypeEdit, localizedText(languageCode, "Task Type", QStringLiteral("任务类型")));
        setFormLabel(basicForm, m_frameworkEdit, localizedText(languageCode, "Framework", QStringLiteral("框架")));
        setFormLabel(basicForm, m_labelsPathEdit, localizedText(languageCode, "Labels Path", QStringLiteral("标签路径")));
    }
    if (auto* inputForm = qobject_cast<QFormLayout*>(m_tabs->widget(1)->layout())) {
        setFormLabel(inputForm, m_inputWidthSpin, localizedText(languageCode, "Width", QStringLiteral("宽度")));
        setFormLabel(inputForm, m_inputHeightSpin, localizedText(languageCode, "Height", QStringLiteral("高度")));
        setFormLabel(inputForm, m_inputChannelsSpin, localizedText(languageCode, "Channels", QStringLiteral("通道数")));
        setFormLabel(inputForm, m_colorOrderCombo, localizedText(languageCode, "Color Order", QStringLiteral("颜色顺序")));
    }
    if (auto* preprocessForm = qobject_cast<QFormLayout*>(m_tabs->widget(2)->layout())) {
        setFormLabel(preprocessForm, m_letterboxCheck, localizedText(languageCode, "Letterbox", QStringLiteral("信箱填充")));
        setFormLabel(preprocessForm, m_keepRatioCheck, localizedText(languageCode, "Keep Ratio", QStringLiteral("保持比例")));
        setFormLabel(preprocessForm, m_scaleSpin, localizedText(languageCode, "Scale", QStringLiteral("缩放系数")));
    }
    if (auto* postForm = qobject_cast<QFormLayout*>(m_tabs->widget(3)->layout())) {
        setFormLabel(postForm, m_confidenceSpin, localizedText(languageCode, "Confidence", QStringLiteral("置信度")));
        setFormLabel(postForm, m_nmsSpin, localizedText(languageCode, "NMS IoU", QStringLiteral("NMS IoU")));
        setFormLabel(postForm, m_maxDetectionsSpin, localizedText(languageCode, "Max Detections", QStringLiteral("最大检测数")));
    }
    if (auto* outputForm = qobject_cast<QFormLayout*>(m_tabs->widget(4)->layout())) {
        setFormLabel(outputForm, m_layoutTypeEdit, localizedText(languageCode, "Output Layout", QStringLiteral("输出布局")));
    }

    if (m_loadTemplateButton) {
        m_loadTemplateButton->setText(localizedText(languageCode, "Load Template", QStringLiteral("加载模板")));
    }
    if (m_saveTemplateButton) {
        m_saveTemplateButton->setText(localizedText(languageCode, "Save As Template", QStringLiteral("另存为模板")));
    }
    if (m_runPreviewButton) {
        m_runPreviewButton->setText(localizedText(languageCode, "Run Preview", QStringLiteral("运行预览")));
    }
    if (m_previewStatusLabel && m_previewSourceImage.empty()) {
        m_previewStatusLabel->setText(localizedText(
            languageCode,
            "Load media in the main window, then run preview here.",
            QStringLiteral("请先在主窗口加载媒体，然后在此运行预览。")));
    }
}

void ModelConfigEditorDialog::handleLoadTemplate()
{
    const QStringList templates = ModelTemplateLoader::listTemplates();
    if (templates.isEmpty()) {
        QMessageBox::information(this, "Load Template", "No templates found in config/model_templates.");
        return;
    }

    bool ok = false;
    const QString templateName = QInputDialog::getItem(
        this,
        "Load Template",
        "Template:",
        templates,
        0,
        false,
        &ok);
    if (!ok || templateName.trimmed().isEmpty()) {
        return;
    }

    DetectionModelDescriptor templateDescriptor;
    QString errorMessage;
    if (!ModelTemplateLoader::loadTemplate(templateName, &templateDescriptor, &errorMessage)) {
        QMessageBox::warning(this, "Load Template", errorMessage);
        return;
    }

    DetectionModelDescriptor current = descriptor();
    ModelTemplateLoader::applyTemplateDefaults(current, templateDescriptor);
    rebuildFromDescriptor(current);
}

void ModelConfigEditorDialog::setDescriptor(const DetectionModelDescriptor& descriptor)
{
    rebuildFromDescriptor(descriptor);
}

DetectionModelDescriptor ModelConfigEditorDialog::descriptor() const
{
    DetectionModelDescriptor descriptor;
    descriptor.modelName = m_modelNameEdit->text().trimmed();
    descriptor.taskType = m_taskTypeEdit->text().trimmed();
    descriptor.frameworkFamily = m_frameworkEdit->text().trimmed();
    descriptor.labelsPath = m_labelsPathEdit->text().trimmed();
    descriptor.input.width = m_inputWidthSpin->value();
    descriptor.input.height = m_inputHeightSpin->value();
    descriptor.input.channels = m_inputChannelsSpin->value();
    descriptor.input.colorOrder = m_colorOrderCombo->currentText();
    descriptor.input.letterbox = m_letterboxCheck->isChecked();
    descriptor.input.keepRatio = m_keepRatioCheck->isChecked();
    descriptor.input.scale = static_cast<float>(m_scaleSpin->value());
    descriptor.postprocess.confidenceThreshold = static_cast<float>(m_confidenceSpin->value());
    descriptor.postprocess.nmsIouThreshold = static_cast<float>(m_nmsSpin->value());
    descriptor.postprocess.maxDetections = m_maxDetectionsSpin->value();
    descriptor.output.layoutType = m_layoutTypeEdit->text().trimmed();
    return descriptor;
}

void ModelConfigEditorDialog::rebuildFromDescriptor(const DetectionModelDescriptor& descriptor)
{
    m_modelNameEdit->setText(descriptor.modelName);
    m_taskTypeEdit->setText(descriptor.taskType);
    m_frameworkEdit->setText(descriptor.frameworkFamily);
    m_labelsPathEdit->setText(descriptor.labelsPath);
    m_inputWidthSpin->setValue(descriptor.input.width);
    m_inputHeightSpin->setValue(descriptor.input.height);
    m_inputChannelsSpin->setValue(descriptor.input.channels);
    m_colorOrderCombo->setCurrentText(descriptor.input.colorOrder);
    m_letterboxCheck->setChecked(descriptor.input.letterbox);
    m_keepRatioCheck->setChecked(descriptor.input.keepRatio);
    m_scaleSpin->setValue(descriptor.input.scale);
    m_confidenceSpin->setValue(descriptor.postprocess.confidenceThreshold);
    m_nmsSpin->setValue(descriptor.postprocess.nmsIouThreshold);
    m_maxDetectionsSpin->setValue(descriptor.postprocess.maxDetections);
    m_layoutTypeEdit->setText(descriptor.output.layoutType);
}

bool ModelConfigEditorDialog::saveDescriptorAsTemplate(const DetectionModelDescriptor& descriptor, QString* errorMessage) const
{
    const QString templateDir = PlatformPaths::resolveResourcePath("config/model_templates");
    QDir().mkpath(templateDir);

    const QString safeName = descriptor.modelName.trimmed().isEmpty()
        ? QString("custom_template")
        : descriptor.modelName.trimmed();
  const QString filePath = QDir(templateDir).filePath(safeName + ".json");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write template: %1").arg(filePath);
        }
        return false;
    }

    QJsonObject root;
    root.insert("template_name", safeName);
    root.insert("task_type", descriptor.taskType);
    root.insert("framework_family", descriptor.frameworkFamily);
    root.insert("labels_path", descriptor.labelsPath);
    root.insert("input", QJsonObject{
        {"width", descriptor.input.width},
        {"height", descriptor.input.height},
        {"channels", descriptor.input.channels},
        {"color_order", descriptor.input.colorOrder},
        {"letterbox", descriptor.input.letterbox},
        {"keep_ratio", descriptor.input.keepRatio},
        {"scale", descriptor.input.scale}
    });
    root.insert("postprocess", QJsonObject{
        {"confidence_threshold", descriptor.postprocess.confidenceThreshold},
        {"nms_iou_threshold", descriptor.postprocess.nmsIouThreshold},
        {"max_detections", descriptor.postprocess.maxDetections}
    });
    root.insert("output", QJsonObject{
        {"layout_type", descriptor.output.layoutType}
    });

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

void ModelConfigEditorDialog::handleSaveTemplate()
{
    const DetectionModelDescriptor current = descriptor();
    QString templateName = QInputDialog::getText(
        this,
        "Save Template",
        "Template name:",
        QLineEdit::Normal,
        current.modelName.isEmpty() ? QString("custom_template") : current.modelName);
    if (templateName.trimmed().isEmpty()) {
        return;
    }

    DetectionModelDescriptor toSave = current;
    toSave.modelName = templateName.trimmed();

    QString errorMessage;
    if (!saveDescriptorAsTemplate(toSave, &errorMessage)) {
        QMessageBox::warning(this, "Save Template", errorMessage);
        return;
    }

    QMessageBox::information(this, "Save Template", QString("Template saved to config/model_templates/%1.json").arg(toSave.modelName));
}

void ModelConfigEditorDialog::setPreviewContext(
    const QString& modelPath,
    const QStringList& labels,
    const cv::Mat& previewImage)
{
    m_previewModelPath = modelPath;
    m_previewLabels = labels;
    m_previewSourceImage = previewImage.clone();
    if (!m_previewSourceImage.empty()) {
        m_previewStatusLabel->setText("Preview source loaded. Adjust config or click Run Preview.");
    }
}

void ModelConfigEditorDialog::connectPreviewSignals()
{
    const auto schedule = [this] { schedulePreviewRefresh(); };
    connect(m_inputWidthSpin, qOverload<int>(&QSpinBox::valueChanged), this, schedule);
    connect(m_inputHeightSpin, qOverload<int>(&QSpinBox::valueChanged), this, schedule);
    connect(m_inputChannelsSpin, qOverload<int>(&QSpinBox::valueChanged), this, schedule);
    connect(m_colorOrderCombo, &QComboBox::currentTextChanged, this, schedule);
    connect(m_letterboxCheck, &QCheckBox::toggled, this, schedule);
    connect(m_keepRatioCheck, &QCheckBox::toggled, this, schedule);
    connect(m_scaleSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, schedule);
    connect(m_confidenceSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, schedule);
    connect(m_nmsSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, schedule);
    connect(m_maxDetectionsSpin, qOverload<int>(&QSpinBox::valueChanged), this, schedule);
    connect(m_layoutTypeEdit, &QLineEdit::textChanged, this, schedule);
    connect(m_taskTypeEdit, &QLineEdit::textChanged, this, schedule);
}

void ModelConfigEditorDialog::schedulePreviewRefresh()
{
    if (m_previewSourceImage.empty() || m_previewModelPath.isEmpty()) {
        return;
    }
    m_previewDebounceTimer->start();
}

void ModelConfigEditorDialog::handleRunPreview()
{
    runPreviewInference();
}

void ModelConfigEditorDialog::runPreviewInference()
{
    if (m_previewSourceImage.empty()) {
        m_previewStatusLabel->setText("No preview image available. Load media in the main window first.");
        return;
    }
    if (m_previewModelPath.isEmpty()) {
        m_previewStatusLabel->setText("Model path is empty.");
        return;
    }

    const DetectionModelDescriptor config = descriptor();
    const auto step = OnnxModelStepFactory::createStep(config, m_previewModelPath, m_previewLabels);
    if (!step) {
        m_previewStatusLabel->setText("Failed to create model step for preview.");
        return;
    }

    FramePacket frame;
    frame.sourceId = "model_config_preview";
    frame.originalMat = m_previewSourceImage;
    frame.workingMat = m_previewSourceImage.clone();

    const StepResult stepResult = step->execute(frame, RunContext{true});
    if (!stepResult.success) {
        m_previewStatusLabel->setText(stepResult.errorMessage);
        return;
    }

    DetectionFrameResult detectionResult;
    detectionResult.sourceId = frame.sourceId;
    if (!frame.annotations.value("detections").isValid()) {
        detectionResult.boxes = {};
    }
    DetectionRenderComposer::applyModelArtifactsOverlay(frame.workingMat, detectionResult, frame.artifacts);
    m_previewCanvas->setImage(OpenCvQtImageConverter::toQImage(frame.workingMat));

    QString summary;
    if (frame.artifacts.contains("classification_top_k")) {
        const QVariantList topK = frame.artifacts.value("classification_top_k").toList();
        if (!topK.isEmpty()) {
            summary = topK.first().toMap().value("label").toString();
        }
    } else if (frame.artifacts.contains("ocr_text")) {
        summary = frame.artifacts.value("ocr_text").toString();
    } else if (frame.artifacts.contains("keypoints")) {
        summary = QString("%1 keypoints").arg(frame.artifacts.value("keypoints").toList().size());
    } else if (frame.metrics.contains("keypoint_count")) {
        summary = QString("%1 keypoints").arg(frame.metrics.value("keypoint_count").toInt());
    } else if (frame.artifacts.contains("custom_tensor_outputs")) {
        summary = QString("%1 tensor output(s)").arg(frame.artifacts.value("custom_tensor_outputs").toList().size());
    }

    m_previewStatusLabel->setText(summary.isEmpty()
        ? "Preview updated."
        : QString("Preview updated: %1").arg(summary));
}
