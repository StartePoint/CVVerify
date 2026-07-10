#include "ui/panels/TaskConsolePanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

bool isChinese(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString taskStateText(TaskState state, bool chinese)
{
    switch (state) {
    case TaskState::Idle:
        return chinese ? QStringLiteral("空闲") : "Idle";
    case TaskState::Running:
        return chinese ? QStringLiteral("运行中") : "Running";
    case TaskState::Paused:
        return chinese ? QStringLiteral("已暂停") : "Paused";
    case TaskState::Completed:
        return chinese ? QStringLiteral("已完成") : "Completed";
    case TaskState::Cancelled:
        return chinese ? QStringLiteral("已取消") : "Cancelled";
    case TaskState::Failed:
        return chinese ? QStringLiteral("失败") : "Failed";
    }
    return chinese ? QStringLiteral("未知") : "Unknown";
}

}

TaskConsolePanel::TaskConsolePanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("taskConsolePanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(6);

    auto* controlRow = new QHBoxLayout();
    controlRow->setSpacing(8);

    m_statusLabel = new QLabel("Idle", this);
    m_statusLabel->setObjectName("taskConsoleStatusLabel");
    m_statusLabel->setMinimumWidth(96);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("taskConsoleProgressBar");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setMinimumHeight(22);
    m_progressBar->setMaximumHeight(22);

    m_cancelButton = new QPushButton("Cancel", this);
    m_pauseButton = new QPushButton("Pause", this);
    m_resumeButton = new QPushButton("Resume", this);
    m_cancelButton->setObjectName("taskCancelButton");
    m_pauseButton->setObjectName("taskPauseButton");
    m_resumeButton->setObjectName("taskResumeButton");
    m_cancelButton->setMinimumHeight(28);
    m_pauseButton->setMinimumHeight(28);
    m_resumeButton->setMinimumHeight(28);

    controlRow->addWidget(m_statusLabel);
    controlRow->addWidget(m_progressBar, 1);
    controlRow->addWidget(m_cancelButton);
    controlRow->addWidget(m_pauseButton);
    controlRow->addWidget(m_resumeButton);

    m_logSummaryLabel = new QLabel(this);
    m_logSummaryLabel->setObjectName("taskConsoleLogSummaryLabel");
    m_logSummaryLabel->setWordWrap(true);
    m_logSummaryLabel->setMaximumHeight(40);
    m_logSummaryLabel->hide();

    layout->addLayout(controlRow);
    layout->addWidget(m_logSummaryLabel);

    connect(m_cancelButton, &QPushButton::clicked, this, &TaskConsolePanel::cancelRequested);
    connect(m_pauseButton, &QPushButton::clicked, this, &TaskConsolePanel::pauseRequested);
    connect(m_resumeButton, &QPushButton::clicked, this, &TaskConsolePanel::resumeRequested);

    setVisible(false);
}

void TaskConsolePanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    rebuildTexts();
}

void TaskConsolePanel::updateProgress(const TaskProgress& progress)
{
    const int percent = progress.total > 0
        ? qBound(0, (progress.current * 100) / progress.total, 100)
        : 0;
    m_progressBar->setValue(percent);
    const bool chinese = isChinese(m_languageCode);
    if (progress.message.isEmpty()) {
        m_statusLabel->setText(
            chinese
                ? QStringLiteral("状态: %1").arg(taskStateText(progress.state, chinese))
                : QString("State: %1").arg(taskStateText(progress.state, chinese)));
    } else {
        m_statusLabel->setText(progress.message);
    }

    const bool running = progress.state == TaskState::Running || progress.state == TaskState::Paused;
    m_cancelButton->setEnabled(running);
    m_pauseButton->setEnabled(progress.state == TaskState::Running);
    m_resumeButton->setEnabled(progress.state == TaskState::Paused);

    const bool hasLog = m_logSummaryLabel && !m_logSummaryLabel->text().isEmpty();
    const bool showPanel = running
        || progress.state == TaskState::Completed
        || progress.state == TaskState::Failed
        || progress.state == TaskState::Cancelled
        || hasLog;
    setVisible(showPanel);
}

void TaskConsolePanel::appendLogSummary(const QString& summary)
{
    if (!m_logSummaryLabel) {
        return;
    }

    m_logSummaryLabel->setText(summary);
    m_logSummaryLabel->setVisible(!summary.isEmpty());
    if (!summary.isEmpty()) {
        setVisible(true);
    }
}

void TaskConsolePanel::rebuildTexts()
{
    const bool chinese = isChinese(m_languageCode);
    m_cancelButton->setText(chinese ? QStringLiteral("取消") : "Cancel");
    m_pauseButton->setText(chinese ? QStringLiteral("暂停") : "Pause");
    m_resumeButton->setText(chinese ? QStringLiteral("继续") : "Resume");
    if (m_statusLabel->text().isEmpty() || m_statusLabel->text().startsWith("State:")
        || m_statusLabel->text().startsWith(QStringLiteral("状态:"))) {
        m_statusLabel->setText(chinese ? QStringLiteral("空闲") : "Idle");
    }
}
