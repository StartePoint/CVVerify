#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;

struct OperatorDescriptor;

class PipelinePanel : public QWidget
{
    Q_OBJECT

public:
    explicit PipelinePanel(QWidget* parent = nullptr);
    void setLanguage(const QString& languageCode);
    void setOperators(const QList<OperatorDescriptor>& descriptors);
    void setPipelineStepNames(const QStringList& stepNames);
    void setCurrentPipelineStep(int index);
    void movePipelineStep(int from, int to);

signals:
    void addOperatorRequested(const QString& operatorId);
    void pipelineStepSelected(int index);
    void removePipelineStepRequested(int index);
    void clearPipelineRequested();
    void savePipelineRequested();
    void loadPipelineRequested();
    void pipelineStepRenamed(int index, const QString& name);
    void pipelineStepMoved(int from, int to);

private:
    bool m_updatingPipelineList = false;
    QString m_languageCode = "en";
    QLabel* m_titleLabel = nullptr;
    QLabel* m_availableLabel = nullptr;
    QLabel* m_pipelineLabel = nullptr;
    QListWidget* m_availableOperatorsList = nullptr;
    QListWidget* m_pipelineStepsList = nullptr;
    QPushButton* m_addOperatorButton = nullptr;
    QPushButton* m_removeOperatorButton = nullptr;
    QPushButton* m_clearPipelineButton = nullptr;
    QPushButton* m_savePipelineButton = nullptr;
    QPushButton* m_loadPipelineButton = nullptr;
};
