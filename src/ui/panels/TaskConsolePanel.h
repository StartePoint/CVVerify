#pragma once

#include <QWidget>

#include "core/tasks/TaskTypes.h"

class QLabel;
class QProgressBar;
class QPushButton;

class TaskConsolePanel : public QWidget
{
    Q_OBJECT

public:
    explicit TaskConsolePanel(QWidget* parent = nullptr);
    void setLanguage(const QString& languageCode);
    void updateProgress(const TaskProgress& progress);
    void appendLogSummary(const QString& summary);

signals:
    void cancelRequested();
    void pauseRequested();
    void resumeRequested();

private:
    void rebuildTexts();

    QString m_languageCode = "en";
    QLabel* m_statusLabel = nullptr;
    QLabel* m_logSummaryLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPushButton* m_cancelButton = nullptr;
    QPushButton* m_pauseButton = nullptr;
    QPushButton* m_resumeButton = nullptr;
};
