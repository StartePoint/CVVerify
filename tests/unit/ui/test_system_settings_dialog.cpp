#include <QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

#include "app/AppSettings.h"
#include "ui/dialogs/SystemSettingsDialog.h"

class SystemSettingsDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void readsSettingsFromEditors();
};

void SystemSettingsDialogTest::readsSettingsFromEditors()
{
    SystemSettingsDialog dialog;

    auto* languageCombo = dialog.findChild<QComboBox*>("languageComboBox");
    auto* exportDirectoryEdit = dialog.findChild<QLineEdit*>("defaultExportDirectoryEdit");
    auto* autoOpenCheckBox = dialog.findChild<QCheckBox*>("autoOpenExportDirectoryCheckBox");
    QVERIFY(languageCombo != nullptr);
    QVERIFY(exportDirectoryEdit != nullptr);
    QVERIFY(autoOpenCheckBox != nullptr);

    languageCombo->setCurrentIndex(languageCombo->findData("zh-CN"));
    exportDirectoryEdit->setText("E:/exports/default");
    autoOpenCheckBox->setChecked(true);

    const AppSettings settings = dialog.settings();

    QCOMPARE(settings.languageCode, QString("zh-CN"));
    QCOMPARE(settings.defaultExportDirectory, QString("E:/exports/default"));
    QCOMPARE(settings.autoOpenExportDirectory, true);
}

int runSystemSettingsDialogTests(int argc, char* argv[])
{
    SystemSettingsDialogTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_system_settings_dialog.moc"
