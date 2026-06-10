#include "ui/dialogs/SystemSettingsDialog.h"

#include <QFileDialog>

#include "ui_SystemSettingsDialog.h"

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

}

SystemSettingsDialog::SystemSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SystemSettingsDialog)
{
    ui->setupUi(this);

    ui->languageComboBox->addItem("English", "en");
    ui->languageComboBox->addItem("Simplified Chinese", "zh-CN");

    connect(ui->browseExportDirectoryButton, &QPushButton::clicked, this, [this] {
        const QString directory = QFileDialog::getExistingDirectory(this, windowTitle(), ui->defaultExportDirectoryEdit->text());
        if (!directory.isEmpty()) {
            ui->defaultExportDirectoryEdit->setText(directory);
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
    const int languageIndex = ui->languageComboBox->findData(appSettings.languageCode);
    ui->languageComboBox->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);
    ui->defaultExportDirectoryEdit->setText(appSettings.defaultExportDirectory);
    ui->autoOpenExportDirectoryCheckBox->setChecked(appSettings.autoOpenExportDirectory);
}

AppSettings SystemSettingsDialog::settings() const
{
    AppSettings appSettings;
    appSettings.languageCode = ui->languageComboBox->currentData().toString();
    appSettings.defaultExportDirectory = ui->defaultExportDirectoryEdit->text().trimmed();
    appSettings.autoOpenExportDirectory = ui->autoOpenExportDirectoryCheckBox->isChecked();
    return appSettings;
}

void SystemSettingsDialog::setLanguage(const QString& languageCode)
{
    if (isChineseLanguage(languageCode)) {
        setWindowTitle(QStringLiteral("系统设置"));
        ui->languageLabel->setText(QStringLiteral("界面语言"));
        ui->defaultExportDirectoryLabel->setText(QStringLiteral("默认导出目录"));
        ui->browseExportDirectoryButton->setText(QStringLiteral("浏览..."));
        ui->autoOpenExportDirectoryCheckBox->setText(QStringLiteral("导出完成后自动打开输出目录"));
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
        ui->browseExportDirectoryButton->setText("Browse...");
        ui->autoOpenExportDirectoryCheckBox->setText("Open output directory automatically after export");
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
