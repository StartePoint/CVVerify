#include "ui/dialogs/SystemSettingsDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "app/AppDefaultsLoader.h"
#include "core/diagnostics/DnnRuntimeInfo.h"
#include "ui_SystemSettingsDialog.h"

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedText(const QString& languageCode, const QString& englishText, const QString& chineseText)
{
    return isChineseLanguage(languageCode) ? chineseText : englishText;
}

}

SystemSettingsDialog::SystemSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SystemSettingsDialog)
{
    ui->setupUi(this);

    ui->languageComboBox->addItem("English", "en");
    ui->languageComboBox->addItem("Simplified Chinese", "zh-CN");

    ui->dnnBackendComboBox->clear();
    const DnnRuntimeInfo::BackendInfo backendInfo = DnnRuntimeInfo::queryBackendInfo();
    for (const QString& backendId : backendInfo.availableBackends) {
        ui->dnnBackendComboBox->addItem(backendId, backendId);
    }
    if (ui->dnnBackendComboBox->count() == 0) {
        ui->dnnBackendComboBox->addItem("cpu", "cpu");
    }

    connect(ui->browseExportDirectoryButton, &QPushButton::clicked, this, [this] {
        const QString directory = QFileDialog::getExistingDirectory(this, windowTitle(), ui->defaultExportDirectoryEdit->text());
        if (!directory.isEmpty()) {
            ui->defaultExportDirectoryEdit->setText(directory);
        }
    });
    connect(ui->resetDefaultsButton, &QPushButton::clicked, this, [this] {
        AppSettings defaults;
        QString errorMessage;
        if (!AppDefaultsLoader::applyTo(&defaults, &errorMessage)) {
            QMessageBox::warning(this, windowTitle(), errorMessage);
            return;
        }
        setSettings(defaults);
    });
    connect(ui->clearRecentFilesButton, &QPushButton::clicked, this, [this] {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this,
            localizedText(m_languageCode, "Clear Recent Files", QStringLiteral("\u6e05\u7a7a\u6700\u8fd1\u6587\u4ef6")),
            localizedText(
                m_languageCode,
                "Clear recent media and project entries when these settings are saved?",
                QStringLiteral("\u4fdd\u5b58\u8bbe\u7f6e\u65f6\u6e05\u7a7a\u6700\u8fd1\u5a92\u4f53\u4e0e\u9879\u76ee\u8bb0\u5f55\u5417\uff1f")
            ),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (answer == QMessageBox::Yes) {
            m_clearRecentFilesRequested = true;
        }
    });

    setLanguage("en");
}

SystemSettingsDialog::~SystemSettingsDialog()
{
    delete ui;
}

void SystemSettingsDialog::setSettings(const AppSettings& appSettings)
{
    m_clearRecentFilesRequested = false;
    const int languageIndex = ui->languageComboBox->findData(appSettings.languageCode);
    ui->languageComboBox->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);
    ui->defaultExportDirectoryEdit->setText(appSettings.defaultExportDirectory);
    ui->autoOpenExportDirectoryCheckBox->setChecked(appSettings.autoOpenExportDirectory);
    ui->maxPreviewFpsSpinBox->setValue(appSettings.maxPreviewFps);
    ui->previewScaleSpinBox->setValue(appSettings.previewScale);
    ui->previewFrameStepSpinBox->setValue(appSettings.previewFrameStep);
    ui->allowFrameSkipCheckBox->setChecked(appSettings.allowFrameSkip);
    ui->offlineStartFrameSpinBox->setValue(appSettings.offlineVideoStartFrame);
    ui->offlineEndFrameSpinBox->setValue(appSettings.offlineVideoEndFrame);
    const int backendIndex = ui->dnnBackendComboBox->findData(appSettings.dnnBackend);
    ui->dnnBackendComboBox->setCurrentIndex(backendIndex >= 0 ? backendIndex : 0);
    ui->exportVideoSideBySideCheckBox->setChecked(appSettings.exportVideoSideBySide);
    ui->showWelcomeGuideCheckBox->setChecked(appSettings.showWelcomeGuide);
    ui->maxRecentFilesSpinBox->setValue(appSettings.maxRecentMediaFiles);
}

AppSettings SystemSettingsDialog::settings() const
{
    AppSettings appSettings;
    appSettings.languageCode = ui->languageComboBox->currentData().toString();
    appSettings.defaultExportDirectory = ui->defaultExportDirectoryEdit->text().trimmed();
    appSettings.autoOpenExportDirectory = ui->autoOpenExportDirectoryCheckBox->isChecked();
    appSettings.maxPreviewFps = ui->maxPreviewFpsSpinBox->value();
    appSettings.previewScale = ui->previewScaleSpinBox->value();
    appSettings.previewFrameStep = ui->previewFrameStepSpinBox->value();
    appSettings.allowFrameSkip = ui->allowFrameSkipCheckBox->isChecked();
    appSettings.offlineVideoStartFrame = ui->offlineStartFrameSpinBox->value();
    appSettings.offlineVideoEndFrame = ui->offlineEndFrameSpinBox->value();
    appSettings.dnnBackend = ui->dnnBackendComboBox->currentData().toString();
    appSettings.exportVideoSideBySide = ui->exportVideoSideBySideCheckBox->isChecked();
    appSettings.showWelcomeGuide = ui->showWelcomeGuideCheckBox->isChecked();
    appSettings.maxRecentMediaFiles = ui->maxRecentFilesSpinBox->value();
    return appSettings;
}

bool SystemSettingsDialog::clearRecentFilesRequested() const
{
    return m_clearRecentFilesRequested;
}

void SystemSettingsDialog::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    if (isChineseLanguage(languageCode)) {
        setWindowTitle(QStringLiteral("系统设置"));
        ui->languageLabel->setText(QStringLiteral("界面语言"));
        ui->defaultExportDirectoryLabel->setText(QStringLiteral("默认导出目录"));
        ui->defaultExportDirectoryHintLabel->setText(
            QStringLiteral("\u7559\u7a7a\u5219\u5728\u5bfc\u51fa\u65f6\u4f7f\u7528\u6e90\u6587\u4ef6\u6240\u5728\u76ee\u5f55\u6216\u5f53\u524d\u76ee\u5f55\u3002")
        );
        ui->browseExportDirectoryButton->setText(QStringLiteral("浏览..."));
        ui->autoOpenExportDirectoryCheckBox->setText(QStringLiteral("导出完成后自动打开输出目录"));
        ui->allowFrameSkipCheckBox->setText(QStringLiteral("播放预览时允许跳帧"));
        ui->maxPreviewFpsLabel->setText(QStringLiteral("预览最大帧率"));
        ui->previewScaleLabel->setText(QStringLiteral("预览缩放比例"));
        ui->previewFrameStepLabel->setText(QStringLiteral("预览帧步进"));
        ui->offlineStartFrameLabel->setText(QStringLiteral("离线视频起始帧"));
        ui->offlineEndFrameLabel->setText(QStringLiteral("离线视频结束帧"));
        ui->dnnBackendLabel->setText(QStringLiteral("DNN 后端"));
        ui->maxRecentFilesLabel->setText(QStringLiteral("最近文件数量上限"));
        ui->exportVideoSideBySideCheckBox->setText(QStringLiteral("导出检测视频时使用左右并排对比"));
        ui->showWelcomeGuideCheckBox->setText(QStringLiteral("下次启动时显示欢迎引导"));
        ui->clearRecentFilesButton->setText(QStringLiteral("\u6e05\u7a7a\u6700\u8fd1\u6587\u4ef6"));
        ui->resetDefaultsButton->setText(QStringLiteral("恢复默认设置"));
        const int englishIndex = ui->languageComboBox->findData("en");
        if (englishIndex >= 0) {
            ui->languageComboBox->setItemText(englishIndex, "English");
        }
        const int chineseIndex = ui->languageComboBox->findData("zh-CN");
        if (chineseIndex >= 0) {
            ui->languageComboBox->setItemText(chineseIndex, QStringLiteral("简体中文"));
        }
    } else {
        setWindowTitle("System Settings");
        ui->languageLabel->setText("Interface Language");
        ui->defaultExportDirectoryLabel->setText("Default Export Directory");
        ui->defaultExportDirectoryHintLabel->setText(
            "Leave empty to use source or current directory for export dialogs."
        );
        ui->browseExportDirectoryButton->setText("Browse...");
        ui->autoOpenExportDirectoryCheckBox->setText("Open output directory automatically after export");
        ui->allowFrameSkipCheckBox->setText("Allow preview frame skipping during playback");
        ui->maxPreviewFpsLabel->setText("Max Preview FPS");
        ui->previewScaleLabel->setText("Preview Scale");
        ui->previewFrameStepLabel->setText("Preview Frame Step");
        ui->offlineStartFrameLabel->setText("Offline Video Start Frame");
        ui->offlineEndFrameLabel->setText("Offline Video End Frame");
        ui->dnnBackendLabel->setText("DNN Backend");
        ui->maxRecentFilesLabel->setText("Max Recent Files");
        ui->exportVideoSideBySideCheckBox->setText("Export detection video as side-by-side comparison");
        ui->showWelcomeGuideCheckBox->setText("Show welcome guide on next launch");
        ui->clearRecentFilesButton->setText("Clear Recent Files");
        ui->resetDefaultsButton->setText("Reset to Defaults");
        const int englishIndex = ui->languageComboBox->findData("en");
        if (englishIndex >= 0) {
            ui->languageComboBox->setItemText(englishIndex, "English");
        }
        const int chineseIndex = ui->languageComboBox->findData("zh-CN");
        if (chineseIndex >= 0) {
            ui->languageComboBox->setItemText(chineseIndex, "Simplified Chinese");
        }
    }
}
