#include "ui/panels/TaskHistoryPanel.h"

#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace {

QString taskKindLabel(TaskKind kind, bool chinese)
{
    switch (kind) {
    case TaskKind::Preview:
        return chinese ? QStringLiteral("预览") : "Preview";
    case TaskKind::BatchImage:
        return chinese ? QStringLiteral("批量图片") : "Batch Image";
    case TaskKind::BatchPipeline:
        return chinese ? QStringLiteral("批量流程") : "Batch Pipeline";
    case TaskKind::OfflineVideo:
        return chinese ? QStringLiteral("离线视频") : "Offline Video";
    }
    return chinese ? QStringLiteral("任务") : "Task";
}

QString taskStateLabel(TaskState state, bool chinese)
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

TaskHistoryPanel::TaskHistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("taskHistoryPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 6, 4, 4);
    layout->setSpacing(8);

    auto* title = new QLabel("Task History", this);
    title->setObjectName("taskHistoryTitleLabel");
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->hide();
    layout->addWidget(title);

    m_taskList = new QListWidget(this);
    m_taskList->setObjectName("taskHistoryList");
    layout->addWidget(m_taskList, 1);

    connect(m_taskList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (!item) {
            return;
        }
        const QString outputDirectory = item->data(Qt::UserRole).toString();
        if (!outputDirectory.isEmpty()) {
            emit openOutputDirectoryRequested(outputDirectory);
        }
    });
}

void TaskHistoryPanel::setTasks(const QList<TaskDefinition>& tasks)
{
    m_cachedTasks = tasks;
    m_taskList->clear();
    const bool chinese = m_languageCode.startsWith("zh", Qt::CaseInsensitive);

    for (const TaskDefinition& task : tasks) {
        const QString line = QString("%1 | %2 | %3")
            .arg(task.displayName.isEmpty() ? task.taskId : task.displayName)
            .arg(taskKindLabel(task.kind, chinese))
            .arg(taskStateLabel(task.state, chinese));
        auto* item = new QListWidgetItem(line, m_taskList);
        item->setData(Qt::UserRole, task.outputDirectory);
        item->setToolTip(task.outputDirectory);
    }
}

void TaskHistoryPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    rebuildTexts();
    setTasks(m_cachedTasks);
}

void TaskHistoryPanel::rebuildTexts()
{
    const bool chinese = m_languageCode.startsWith("zh", Qt::CaseInsensitive);
    if (auto* title = findChild<QLabel*>("taskHistoryTitleLabel")) {
        title->setText(chinese ? QStringLiteral("任务历史") : "Task History");
    }
}
