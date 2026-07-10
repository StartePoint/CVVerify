#pragma once

#include <QWidget>

class QPlainTextEdit;
class QPushButton;

class DiagnosticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DiagnosticsPanel(QWidget* parent = nullptr);

public slots:
    void refresh();
    void setLanguage(const QString& languageCode);

private:
    QPlainTextEdit* m_textEdit = nullptr;
    QPushButton* m_clearButton = nullptr;
    QString m_languageCode = "en";
};
