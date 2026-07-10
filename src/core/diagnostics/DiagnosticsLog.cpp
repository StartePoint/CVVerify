#include "core/diagnostics/DiagnosticsLog.h"

#include <algorithm>

#include <QMutexLocker>

DiagnosticsLog& DiagnosticsLog::instance()
{
    static DiagnosticsLog log;
    return log;
}

void DiagnosticsLog::clear()
{
    QMutexLocker locker(&m_mutex);
    m_entries.clear();
    m_stepTimings.clear();
    m_tensorShapes.clear();
    m_runtimeBackend.clear();
    m_runtimeTarget.clear();
}

void DiagnosticsLog::log(LogLevel level, const QString& category, const QString& message)
{
    QMutexLocker locker(&m_mutex);
    m_entries.append({QDateTime::currentDateTime(), level, category, message});
}

void DiagnosticsLog::info(const QString& category, const QString& message)
{
    log(LogLevel::Info, category, message);
}

void DiagnosticsLog::warning(const QString& category, const QString& message)
{
    log(LogLevel::Warning, category, message);
}

void DiagnosticsLog::error(const QString& category, const QString& message)
{
    log(LogLevel::Error, category, message);
}

void DiagnosticsLog::recordStepTiming(const QString& stepId, const QString& displayName, qint64 durationMs)
{
    QMutexLocker locker(&m_mutex);
    m_stepTimings.append({stepId, displayName, durationMs});
}

void DiagnosticsLog::clearStepTimings()
{
    QMutexLocker locker(&m_mutex);
    m_stepTimings.clear();
}

void DiagnosticsLog::recordTensorShape(const QString& modelName, const QString& tensorName, const QString& shapeText, bool isInput)
{
    QMutexLocker locker(&m_mutex);
    m_tensorShapes.append({modelName, tensorName, shapeText, isInput});
}

void DiagnosticsLog::recordRuntimeBackend(const QString& backendName, const QString& targetName)
{
    QMutexLocker locker(&m_mutex);
    m_runtimeBackend = backendName;
    m_runtimeTarget = targetName;
}

QList<TensorShapeEntry> DiagnosticsLog::tensorShapes() const
{
    QMutexLocker locker(&m_mutex);
    return m_tensorShapes;
}

QString DiagnosticsLog::runtimeBackendSummary() const
{
    QMutexLocker locker(&m_mutex);
    if (m_runtimeBackend.isEmpty()) {
        return QString();
    }
    return QString("%1 / %2").arg(m_runtimeBackend, m_runtimeTarget);
}

QList<LogEntry> DiagnosticsLog::entries() const
{
    QMutexLocker locker(&m_mutex);
    return m_entries;
}

QList<StepTimingEntry> DiagnosticsLog::stepTimings() const
{
    QMutexLocker locker(&m_mutex);
    return m_stepTimings;
}

qint64 DiagnosticsLog::totalStepTimingMs() const
{
    QMutexLocker locker(&m_mutex);
    qint64 total = 0;
    for (const StepTimingEntry& entry : m_stepTimings) {
        total += entry.durationMs;
    }
    return total;
}

QString DiagnosticsLog::summaryText() const
{
    QMutexLocker locker(&m_mutex);
    const int errorCount = std::count_if(m_entries.begin(), m_entries.end(), [](const LogEntry& entry) {
        return entry.level == LogLevel::Error;
    });
    const int warningCount = std::count_if(m_entries.begin(), m_entries.end(), [](const LogEntry& entry) {
        return entry.level == LogLevel::Warning;
    });

    qint64 totalMs = 0;
    for (const StepTimingEntry& entry : m_stepTimings) {
        totalMs += entry.durationMs;
    }

    return QString("Logs: %1 | Warnings: %2 | Errors: %3 | Step time: %4 ms")
        .arg(m_entries.size())
        .arg(warningCount)
        .arg(errorCount)
        .arg(totalMs);
}

QString DiagnosticsLog::detailedText() const
{
    QMutexLocker locker(&m_mutex);
    QStringList lines;
    lines << summaryText();
    if (!m_runtimeBackend.isEmpty()) {
        lines << QString("Runtime backend: %1 / %2").arg(m_runtimeBackend, m_runtimeTarget);
    }
    for (const TensorShapeEntry& tensor : m_tensorShapes) {
        lines << QString("%1 %2: %3")
                     .arg(tensor.isInput ? "Input" : "Output", tensor.tensorName, tensor.shapeText);
    }
    for (const StepTimingEntry& timing : m_stepTimings) {
        lines << QString("%1: %2 ms").arg(timing.displayName).arg(timing.durationMs);
    }
    for (const LogEntry& entry : m_entries) {
        const QString level = entry.level == LogLevel::Error ? "ERROR"
            : entry.level == LogLevel::Warning ? "WARN" : "INFO";
        lines << QString("[%1][%2] %3").arg(level, entry.category, entry.message);
    }
    return lines.join('\n');
}
