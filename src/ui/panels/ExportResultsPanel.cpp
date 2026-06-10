#include "ui/panels/ExportResultsPanel.h"

#include "ui_ExportResultsPanel.h"

namespace {

bool isChineseLanguageCode(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

}

ExportResultsPanel::ExportResultsPanel(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ExportResultsPanel)
{
    ui->setupUi(this);

    connect(ui->openExportDirectoryButton, &QPushButton::clicked, this, [this] {
        if (!m_latestSummary.outputDirectory.isEmpty()) {
            emit openOutputDirectoryRequested(m_latestSummary.outputDirectory);
        }
    });
    connect(ui->openSelectedArtifactButton, &QPushButton::clicked, this, [this] {
        emitOpenSelectedArtifact();
    });
    connect(ui->exportArtifactsList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        emitOpenSelectedArtifact();
    });
    connect(ui->exportHistoryList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row < 0 || row >= m_history.size()) {
            return;
        }

        m_latestSummary = m_history.at(row);
        showSummary(m_latestSummary);
    });

    setLanguage("en");
    clearSummary();
}

ExportResultsPanel::~ExportResultsPanel()
{
    delete ui;
}

void ExportResultsPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = isChineseLanguage();
    ui->panelTitleLabel->setText(chinese ? QStringLiteral("导出结果") : "Export Results");
    ui->historyTitleLabel->setText(chinese ? QStringLiteral("最近导出") : "Recent Exports");
    ui->artifactsTitleLabel->setText(chinese ? QStringLiteral("产物") : "Artifacts");
    ui->openExportDirectoryButton->setText(chinese ? QStringLiteral("打开输出目录") : "Open Output Directory");
    ui->openSelectedArtifactButton->setText(chinese ? QStringLiteral("打开选中产物") : "Open Selected Artifact");

    if (m_latestSummary.title.isEmpty()) {
        ui->exportTitleLabel->setText(chinese ? QStringLiteral("最新导出") : "Latest Export");
        ui->exportSummaryLabel->setText(chinese ? QStringLiteral("当前还没有导出记录。") : "No export has been recorded yet.");
        ui->exportOutputDirLabel->setText(chinese ? QStringLiteral("输出目录: -") : "Output Directory: -");
    } else {
        showSummary(m_latestSummary);
    }
}

void ExportResultsPanel::setLatestSummary(const ExportResultSummary& summary)
{
    m_latestSummary = summary;

    m_history.prepend(summary);
    while (m_history.size() > 10) {
        m_history.removeLast();
    }

    ui->exportHistoryList->clear();
    for (const ExportResultSummary& item : m_history) {
        ui->exportHistoryList->addItem(item.title.isEmpty()
            ? (isChineseLanguage() ? QStringLiteral("最新导出") : QString("Latest Export"))
            : item.title);
    }
    ui->exportHistoryList->setCurrentRow(0);

    showSummary(summary);
}

void ExportResultsPanel::showSummary(const ExportResultSummary& summary)
{
    const bool chinese = isChineseLanguage();
    ui->exportTitleLabel->setText(summary.title.isEmpty()
        ? (chinese ? QStringLiteral("最新导出") : QString("Latest Export"))
        : summary.title);
    ui->exportSummaryLabel->setText(summary.summaryText.isEmpty()
        ? (chinese ? QStringLiteral("暂无导出摘要。") : QString("No export summary available."))
        : summary.summaryText);
    ui->exportOutputDirLabel->setText(summary.outputDirectory.isEmpty()
        ? (chinese ? QStringLiteral("输出目录: -") : QString("Output Directory: -"))
        : (chinese
            ? QStringLiteral("输出目录: %1").arg(summary.outputDirectory)
            : QString("Output Directory: %1").arg(summary.outputDirectory)));
    ui->exportArtifactsList->clear();
    for (const QString& artifact : summary.artifacts) {
        ui->exportArtifactsList->addItem(artifact);
    }

    ui->openExportDirectoryButton->setEnabled(!summary.outputDirectory.isEmpty());
    ui->openSelectedArtifactButton->setEnabled(!summary.outputDirectory.isEmpty() && !summary.artifacts.isEmpty());
}

void ExportResultsPanel::clearSummary()
{
    m_history.clear();
    m_latestSummary = {};
    ui->exportHistoryList->clear();
    setLanguage(m_languageCode);
    ui->exportArtifactsList->clear();
    ui->openExportDirectoryButton->setEnabled(false);
    ui->openSelectedArtifactButton->setEnabled(false);
}

void ExportResultsPanel::emitOpenSelectedArtifact()
{
    auto* item = ui->exportArtifactsList->currentItem();
    if (!item || m_latestSummary.outputDirectory.isEmpty()) {
        return;
    }

    emit openArtifactRequested(m_latestSummary.outputDirectory, item->text());
}

bool ExportResultsPanel::isChineseLanguage() const
{
    return isChineseLanguageCode(m_languageCode);
}
