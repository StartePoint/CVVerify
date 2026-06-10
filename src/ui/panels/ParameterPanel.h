#pragma once

#include <memory>

#include <QList>
#include <QVariantMap>
#include <QWidget>

class QFormLayout;
class QLabel;
class QScrollArea;
class QWidget;

class IPipelineStep;

class ParameterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget* parent = nullptr);
    void setLanguage(const QString& languageCode);
    void setStep(const std::shared_ptr<IPipelineStep>& step);
    void clearStep();

signals:
    void parameterValuesChanged(const QVariantMap& values);

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
    QString m_languageCode = "en";
    QList<ParameterEditorBinding> m_editorBindings;
};
