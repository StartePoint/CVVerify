#pragma once

#include <QObject>

#include "core/tasks/TaskTypes.h"

class QThread;

class TaskWorker : public QObject
{
    Q_OBJECT

public:
    explicit TaskWorker(TaskKind kind, int totalItems, TaskWorkFunction workFunction, QObject* parent = nullptr);

public slots:
    void run();
    void requestCancel();
    void requestPause();
    void requestResume();

signals:
    void progressChanged(const TaskProgress& progress);
    void finished(bool success);

private:
    TaskKind m_kind = TaskKind::Preview;
    int m_totalItems = 0;
    TaskWorkFunction m_workFunction;
    bool m_cancelRequested = false;
    bool m_pauseRequested = false;
};

class TaskScheduler : public QObject
{
    Q_OBJECT

public:
    explicit TaskScheduler(QObject* parent = nullptr);
    ~TaskScheduler() override;

    bool isRunning() const;
    TaskProgress currentProgress() const;

    bool startTask(TaskKind kind, int totalItems, TaskWorkFunction workFunction);
    void cancelTask();
    void pauseTask();
    void resumeTask();

signals:
    void progressChanged(const TaskProgress& progress);
    void taskFinished(bool success);

private:
    QThread* m_thread = nullptr;
    TaskWorker* m_worker = nullptr;
    TaskProgress m_progress;
};
