#pragma once

#include <QDialog>

#include "app/AppSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class SystemSettingsDialog;
}
QT_END_NAMESPACE

class SystemSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemSettingsDialog(QWidget* parent = nullptr);
    ~SystemSettingsDialog() override;

    void setSettings(const AppSettings& settings);
    AppSettings settings() const;
    void setLanguage(const QString& languageCode);
    bool clearRecentFilesRequested() const;

private:
    Ui::SystemSettingsDialog* ui = nullptr;
    QString m_languageCode = "en";
    bool m_clearRecentFilesRequested = false;
};
