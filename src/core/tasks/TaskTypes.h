#pragma once

#include <functional>

#include <QDateTime>
#include <QString>

enum class TaskState
{
    Idle,
    Running,
    Paused,
    Completed,
    Cancelled,
    Failed
};

enum class TaskKind
{
    Preview,
    BatchImage,
    BatchPipeline,
    OfflineVideo
};

struct TaskProgress
{
    TaskState state = TaskState::Idle;
    TaskKind kind = TaskKind::Preview;
    int current = 0;
    int total = 0;
    QString message;
    QString errorMessage;
    qint64 elapsedMs = 0;
    qint64 estimatedRemainingMs = -1;
};

struct TaskDefinition
{
    QString taskId;
    QString displayName;
    TaskKind kind = TaskKind::Preview;
    TaskState state = TaskState::Idle;
    TaskProgress progress;
    QDateTime createdAt;
    QDateTime finishedAt;
    QString outputDirectory;
};

using TaskWorkFunction = std::function<bool(int current, int total, bool* cancelRequested, bool* pauseRequested, QString* errorMessage)>;
