#include "core/tasks/TaskScheduler.h"

#include <QThread>

#include "core/diagnostics/DiagnosticsLog.h"

TaskWorker::TaskWorker(TaskKind kind, int totalItems, TaskWorkFunction workFunction, QObject* parent)
    : QObject(parent)
    , m_kind(kind)
    , m_totalItems(totalItems)
    , m_workFunction(std::move(workFunction))
{
}

void TaskWorker::run()
{
    TaskProgress progress;
    progress.state = TaskState::Running;
    progress.kind = m_kind;
    progress.total = m_totalItems;

    DiagnosticsLog::instance().info("Task", QString("Task started: total=%1").arg(m_totalItems));

    for (int index = 0; index < m_totalItems; ++index) {
        while (m_pauseRequested && !m_cancelRequested) {
            progress.state = TaskState::Paused;
            progress.current = index;
            progress.message = "Paused";
            emit progressChanged(progress);
            QThread::msleep(50);
        }

        if (m_cancelRequested) {
            progress.state = TaskState::Cancelled;
            progress.current = index;
            progress.message = "Cancelled";
            emit progressChanged(progress);
            DiagnosticsLog::instance().warning("Task", "Task cancelled by user");
            emit finished(false);
            return;
        }

        progress.state = TaskState::Running;
        progress.current = index + 1;
        progress.message = QString("Processing %1/%2").arg(index + 1).arg(m_totalItems);
        emit progressChanged(progress);

        QString errorMessage;
        if (!m_workFunction(index, m_totalItems, &m_cancelRequested, &m_pauseRequested, &errorMessage)) {
            progress.state = TaskState::Failed;
            progress.errorMessage = errorMessage;
            progress.message = errorMessage.isEmpty() ? "Task failed" : errorMessage;
            emit progressChanged(progress);
            DiagnosticsLog::instance().error("Task", progress.message);
            emit finished(false);
            return;
        }
    }

    progress.state = TaskState::Completed;
    progress.current = m_totalItems;
    progress.message = "Completed";
    emit progressChanged(progress);
    DiagnosticsLog::instance().info("Task", "Task completed");
    emit finished(true);
}

void TaskWorker::requestCancel()
{
    m_cancelRequested = true;
    m_pauseRequested = false;
}

void TaskWorker::requestPause()
{
    m_pauseRequested = true;
}

void TaskWorker::requestResume()
{
    m_pauseRequested = false;
}

TaskScheduler::TaskScheduler(QObject* parent)
    : QObject(parent)
{
}

TaskScheduler::~TaskScheduler()
{
    cancelTask();
}

bool TaskScheduler::isRunning() const
{
    return m_thread != nullptr;
}

TaskProgress TaskScheduler::currentProgress() const
{
    return m_progress;
}

bool TaskScheduler::startTask(TaskKind kind, int totalItems, TaskWorkFunction workFunction)
{
    if (m_thread || totalItems <= 0 || !workFunction) {
        return false;
    }

    m_progress = {};
    m_progress.state = TaskState::Running;
    m_progress.kind = kind;
    m_progress.total = totalItems;

    m_thread = new QThread(this);
    m_worker = new TaskWorker(kind, totalItems, std::move(workFunction));
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &TaskWorker::run);
    connect(m_worker, &TaskWorker::progressChanged, this, [this](const TaskProgress& progress) {
        m_progress = progress;
        emit progressChanged(progress);
    });
    connect(m_worker, &TaskWorker::finished, this, [this](bool success) {
        m_thread->quit();
        m_thread->wait();
        m_worker->deleteLater();
        m_worker = nullptr;
        m_thread->deleteLater();
        m_thread = nullptr;
        emit taskFinished(success);
    });

    m_thread->start();
    return true;
}

void TaskScheduler::cancelTask()
{
    if (m_worker) {
        m_worker->requestCancel();
    }
}

void TaskScheduler::pauseTask()
{
    if (m_worker) {
        m_worker->requestPause();
    }
}

void TaskScheduler::resumeTask()
{
    if (m_worker) {
        m_worker->requestResume();
    }
}
