#pragma once

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QString>

enum class LogLevel
{
    Info,
    Warning,
    Error
};

struct LogEntry
{
    QDateTime timestamp;
    LogLevel level = LogLevel::Info;
    QString category;
    QString message;
};

struct StepTimingEntry
{
    QString stepId;
    QString displayName;
    qint64 durationMs = 0;
};

struct TensorShapeEntry
{
    QString modelName;
    QString tensorName;
    QString shapeText;
    bool isInput = true;
};

class DiagnosticsLog
{
public:
    static DiagnosticsLog& instance();

    void clear();
    void log(LogLevel level, const QString& category, const QString& message);
    void info(const QString& category, const QString& message);
    void warning(const QString& category, const QString& message);
    void error(const QString& category, const QString& message);

    void recordStepTiming(const QString& stepId, const QString& displayName, qint64 durationMs);
    void clearStepTimings();
    void recordTensorShape(const QString& modelName, const QString& tensorName, const QString& shapeText, bool isInput);
    void recordRuntimeBackend(const QString& backendName, const QString& targetName);

    QList<LogEntry> entries() const;
    QList<StepTimingEntry> stepTimings() const;
    QList<TensorShapeEntry> tensorShapes() const;
    QString runtimeBackendSummary() const;
    qint64 totalStepTimingMs() const;
    QString summaryText() const;
    QString detailedText() const;

private:
    DiagnosticsLog() = default;

    mutable QMutex m_mutex;
    QList<LogEntry> m_entries;
    QList<StepTimingEntry> m_stepTimings;
    QList<TensorShapeEntry> m_tensorShapes;
    QString m_runtimeBackend;
    QString m_runtimeTarget;
};
