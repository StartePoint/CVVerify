#pragma once

#include <QWidget>

#include "core/tasks/TaskTypes.h"

class QListWidget;

class TaskHistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TaskHistoryPanel(QWidget* parent = nullptr);

    void setTasks(const QList<TaskDefinition>& tasks);
    void setLanguage(const QString& languageCode);

signals:
    void openOutputDirectoryRequested(const QString& outputDirectory);

private:
    void rebuildTexts();

    QListWidget* m_taskList = nullptr;
    QList<TaskDefinition> m_cachedTasks;
    QString m_languageCode = "en";
};
