#pragma once

#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;

class ModelListPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ModelListPanel(QWidget* parent = nullptr);

    void refreshModels();
    void setLanguage(const QString& languageCode);

signals:
    void modelActivated(const QString& modelName);
    void editModelRequested(const QString& modelName);

private slots:
    void handleSelectionChanged();
    void handleActivateClicked();
    void handleEditClicked();

private:
    QLabel* m_titleLabel = nullptr;
    QListWidget* m_modelList = nullptr;
    QPushButton* m_activateButton = nullptr;
    QPushButton* m_editButton = nullptr;
    QString m_languageCode = "en";
};
