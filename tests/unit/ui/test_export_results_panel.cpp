#include <QtTest>

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>

#include "ui/panels/ExportResultsPanel.h"

class ExportResultsPanelTest : public QObject
{
    Q_OBJECT

private slots:
    void showsLatestExportSummary();
    void emitsOpenActionsForLatestSummary();
    void browsesRecentExportHistory();
};

void ExportResultsPanelTest::showsLatestExportSummary()
{
    ExportResultsPanel panel;

    ExportResultSummary summary;
    summary.title = "Batch Image Export";
    summary.summaryText = "Exported 3 image detection result(s)";
    summary.outputDirectory = "E:/exports/demo";
    summary.artifacts = QStringList() << "detections.csv" << "pipeline_snapshot.json";

    panel.setLatestSummary(summary);

    auto* titleLabel = panel.findChild<QLabel*>("exportTitleLabel");
    auto* summaryLabel = panel.findChild<QLabel*>("exportSummaryLabel");
    auto* outputDirLabel = panel.findChild<QLabel*>("exportOutputDirLabel");
    auto* artifactsList = panel.findChild<QListWidget*>("exportArtifactsList");
    QVERIFY(titleLabel != nullptr);
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(outputDirLabel != nullptr);
    QVERIFY(artifactsList != nullptr);

    QCOMPARE(titleLabel->text(), QString("Batch Image Export"));
    QCOMPARE(summaryLabel->text(), QString("Exported 3 image detection result(s)"));
    QVERIFY(outputDirLabel->text().contains("E:/exports/demo"));
    QCOMPARE(artifactsList->count(), 2);
    QCOMPARE(artifactsList->item(0)->text(), QString("detections.csv"));
}

void ExportResultsPanelTest::emitsOpenActionsForLatestSummary()
{
    ExportResultsPanel panel;

    ExportResultSummary summary;
    summary.title = "Video Export";
    summary.summaryText = "Exported detection video";
    summary.outputDirectory = "E:/exports/video_run";
    summary.artifacts = QStringList() << "sample_overlay.avi" << "sample.json";

    panel.setLatestSummary(summary);

    auto* openDirButton = panel.findChild<QPushButton*>("openExportDirectoryButton");
    auto* openArtifactButton = panel.findChild<QPushButton*>("openSelectedArtifactButton");
    auto* artifactsList = panel.findChild<QListWidget*>("exportArtifactsList");
    QVERIFY(openDirButton != nullptr);
    QVERIFY(openArtifactButton != nullptr);
    QVERIFY(artifactsList != nullptr);

    QSignalSpy directorySpy(&panel, SIGNAL(openOutputDirectoryRequested(QString)));
    QSignalSpy artifactSpy(&panel, SIGNAL(openArtifactRequested(QString,QString)));

    openDirButton->click();
    QCOMPARE(directorySpy.count(), 1);
    QCOMPARE(directorySpy.takeFirst().at(0).toString(), QString("E:/exports/video_run"));

    artifactsList->setCurrentRow(1);
    openArtifactButton->click();

    QCOMPARE(artifactSpy.count(), 1);
    const QList<QVariant> payload = artifactSpy.takeFirst();
    QCOMPARE(payload.at(0).toString(), QString("E:/exports/video_run"));
    QCOMPARE(payload.at(1).toString(), QString("sample.json"));
}

void ExportResultsPanelTest::browsesRecentExportHistory()
{
    ExportResultsPanel panel;

    ExportResultSummary firstSummary;
    firstSummary.title = "Batch Image Export";
    firstSummary.summaryText = "Exported 3 image detection result(s)";
    firstSummary.outputDirectory = "E:/exports/batch";
    firstSummary.artifacts = QStringList() << "detections.csv";

    ExportResultSummary secondSummary;
    secondSummary.title = "Video Export";
    secondSummary.summaryText = "Exported detection video";
    secondSummary.outputDirectory = "E:/exports/video";
    secondSummary.artifacts = QStringList() << "sample_overlay.avi" << "sample.csv";

    panel.setLatestSummary(firstSummary);
    panel.setLatestSummary(secondSummary);

    auto* historyList = panel.findChild<QListWidget*>("exportHistoryList");
    auto* titleLabel = panel.findChild<QLabel*>("exportTitleLabel");
    auto* outputDirLabel = panel.findChild<QLabel*>("exportOutputDirLabel");
    auto* artifactsList = panel.findChild<QListWidget*>("exportArtifactsList");
    QVERIFY(historyList != nullptr);
    QVERIFY(titleLabel != nullptr);
    QVERIFY(outputDirLabel != nullptr);
    QVERIFY(artifactsList != nullptr);

    QCOMPARE(historyList->count(), 2);
    QCOMPARE(titleLabel->text(), QString("Video Export"));
    QVERIFY(outputDirLabel->text().contains("E:/exports/video"));
    QCOMPARE(artifactsList->count(), 2);

    historyList->setCurrentRow(1);

    QCOMPARE(titleLabel->text(), QString("Batch Image Export"));
    QVERIFY(outputDirLabel->text().contains("E:/exports/batch"));
    QCOMPARE(artifactsList->count(), 1);
    QCOMPARE(artifactsList->item(0)->text(), QString("detections.csv"));
}

int runExportResultsPanelTests(int argc, char* argv[])
{
    ExportResultsPanelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_export_results_panel.moc"
