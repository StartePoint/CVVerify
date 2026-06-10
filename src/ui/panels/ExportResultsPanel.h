#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

struct ExportResultSummary
{
    QString title;
    QString summaryText;
    QString outputDirectory;
    QStringList artifacts;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class ExportResultsPanel;
}
QT_END_NAMESPACE

class ExportResultsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ExportResultsPanel(QWidget* parent = nullptr);
    ~ExportResultsPanel() override;

    void setLanguage(const QString& languageCode);
    void setLatestSummary(const ExportResultSummary& summary);
    void clearSummary();

signals:
    void openOutputDirectoryRequested(const QString& outputDirectory);
    void openArtifactRequested(const QString& outputDirectory, const QString& artifactName);

private:
    void emitOpenSelectedArtifact();
    void showSummary(const ExportResultSummary& summary);
    bool isChineseLanguage() const;

    QList<ExportResultSummary> m_history;
    QString m_languageCode = "en";
    ExportResultSummary m_latestSummary;
    Ui::ExportResultsPanel* ui = nullptr;
};
