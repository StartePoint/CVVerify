#include <QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTest>

#include "app/AppDefaultsLoader.h"
#include "app/AppSettings.h"
#include "ui/dialogs/SystemSettingsDialog.h"

class SystemSettingsDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void readsSettingsFromEditors();
    void resetsEditorsToBundledDefaults();
    void exposesClearRecentFilesControls();
};

void SystemSettingsDialogTest::readsSettingsFromEditors()
{
    SystemSettingsDialog dialog;

    auto* languageCombo = dialog.findChild<QComboBox*>("languageComboBox");
    auto* exportDirectoryEdit = dialog.findChild<QLineEdit*>("defaultExportDirectoryEdit");
    auto* autoOpenCheckBox = dialog.findChild<QCheckBox*>("autoOpenExportDirectoryCheckBox");
    auto* maxPreviewFpsSpin = dialog.findChild<QSpinBox*>("maxPreviewFpsSpinBox");
    auto* previewScaleSpin = dialog.findChild<QDoubleSpinBox*>("previewScaleSpinBox");
    auto* dnnBackendCombo = dialog.findChild<QComboBox*>("dnnBackendComboBox");
    auto* exportVideoSideBySideCheckBox = dialog.findChild<QCheckBox*>("exportVideoSideBySideCheckBox");
    auto* showWelcomeGuideCheckBox = dialog.findChild<QCheckBox*>("showWelcomeGuideCheckBox");
    auto* maxRecentFilesSpin = dialog.findChild<QSpinBox*>("maxRecentFilesSpinBox");
    QVERIFY(languageCombo != nullptr);
    QVERIFY(exportDirectoryEdit != nullptr);
    QVERIFY(autoOpenCheckBox != nullptr);
    QVERIFY(maxPreviewFpsSpin != nullptr);
    QVERIFY(previewScaleSpin != nullptr);
    QVERIFY(dnnBackendCombo != nullptr);
    QVERIFY(exportVideoSideBySideCheckBox != nullptr);
    QVERIFY(showWelcomeGuideCheckBox != nullptr);
    QVERIFY(maxRecentFilesSpin != nullptr);

    languageCombo->setCurrentIndex(languageCombo->findData("zh-CN"));
    exportDirectoryEdit->setText("E:/exports/default");
    autoOpenCheckBox->setChecked(true);
    maxPreviewFpsSpin->setValue(15);
    previewScaleSpin->setValue(0.5);
    const int openclIndex = dnnBackendCombo->findData("opencl");
    if (openclIndex >= 0) {
        dnnBackendCombo->setCurrentIndex(openclIndex);
    }
    exportVideoSideBySideCheckBox->setChecked(true);
    showWelcomeGuideCheckBox->setChecked(false);
    maxRecentFilesSpin->setValue(15);

    const AppSettings settings = dialog.settings();

    QCOMPARE(settings.languageCode, QString("zh-CN"));
    QCOMPARE(settings.defaultExportDirectory, QString("E:/exports/default"));
    QCOMPARE(settings.autoOpenExportDirectory, true);
    QCOMPARE(settings.maxPreviewFps, 15);
    QCOMPARE(settings.previewScale, 0.5);
    if (openclIndex >= 0) {
        QCOMPARE(settings.dnnBackend, QString("opencl"));
    }
    QCOMPARE(settings.exportVideoSideBySide, true);
    QCOMPARE(settings.showWelcomeGuide, false);
    QCOMPARE(settings.maxRecentMediaFiles, 15);
}

void SystemSettingsDialogTest::resetsEditorsToBundledDefaults()
{
    SystemSettingsDialog dialog;

    AppSettings custom;
    custom.languageCode = "zh-CN";
    custom.maxPreviewFps = 99;
    custom.previewScale = 0.25;
    custom.showWelcomeGuide = false;
    custom.maxRecentMediaFiles = 25;
    dialog.setSettings(custom);

    auto* resetButton = dialog.findChild<QPushButton*>("resetDefaultsButton");
    QVERIFY(resetButton != nullptr);
    QTest::mouseClick(resetButton, Qt::LeftButton);

    AppSettings defaults;
    QVERIFY(AppDefaultsLoader::applyTo(&defaults));

    const AppSettings settings = dialog.settings();
    QCOMPARE(settings.languageCode, defaults.languageCode);
    QCOMPARE(settings.maxPreviewFps, defaults.maxPreviewFps);
    QCOMPARE(settings.previewScale, defaults.previewScale);
    QCOMPARE(settings.showWelcomeGuide, defaults.showWelcomeGuide);
    QCOMPARE(settings.maxRecentMediaFiles, defaults.maxRecentMediaFiles);
}

void SystemSettingsDialogTest::exposesClearRecentFilesControls()
{
    SystemSettingsDialog dialog;
    auto* clearButton = dialog.findChild<QPushButton*>("clearRecentFilesButton");
    auto* hintLabel = dialog.findChild<QLabel*>("defaultExportDirectoryHintLabel");
    QVERIFY(clearButton != nullptr);
    QVERIFY(hintLabel != nullptr);
    QVERIFY(!hintLabel->text().isEmpty());
    QVERIFY(!dialog.clearRecentFilesRequested());
}

int runSystemSettingsDialogTests(int argc, char* argv[])
{
    SystemSettingsDialogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_system_settings_dialog.moc"
