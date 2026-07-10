#pragma once

#include <memory>

#include <QList>
#include <QVariantMap>
#include <QWidget>

class QFormLayout;
class QLabel;
class QPushButton;
class QScrollArea;
class QWidget;

class IPipelineStep;

class ParameterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget* parent = nullptr);
    void setLanguage(const QString& languageCode);
    void setCompactMode(bool compact);
    void setStep(const std::shared_ptr<IPipelineStep>& step);
    void clearStep();

signals:
    void parameterValuesChanged(const QVariantMap& values);
    void resetToDefaultsRequested();
    void savePresetRequested();
    void loadPresetRequested();

private:
    struct ParameterEditorBinding
    {
        QString parameterKey;
        QString visibleWhen;
        QWidget* labelWidget = nullptr;
        QWidget* rowWidget = nullptr;
    };

    void rebuildEditors();
    void updateConditionalVisibility(const QVariantMap& values);
    bool evaluateVisibleWhen(const QString& visibleWhen, const QVariantMap& values) const;

    std::shared_ptr<IPipelineStep> m_step;
    QFormLayout* m_formLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_emptyStateLabel = nullptr;
    QPushButton* m_resetButton = nullptr;
    QPushButton* m_savePresetButton = nullptr;
    QPushButton* m_loadPresetButton = nullptr;
    QString m_languageCode = "en";
    bool m_compactMode = false;
    bool m_rebuildingEditors = false;
    QList<ParameterEditorBinding> m_editorBindings;
};
