#pragma once

#include <memory>

#include <QDialog>

class ParameterPanel;
class IPipelineStep;

QT_BEGIN_NAMESPACE
namespace Ui {
class OperatorParameterDialog;
}
QT_END_NAMESPACE

class OperatorParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OperatorParameterDialog(QWidget* parent = nullptr);
    ~OperatorParameterDialog() override;

    void setLanguage(const QString& languageCode);
    void setOperatorStep(const QString& operatorId, const QString& displayName, const std::shared_ptr<IPipelineStep>& step);
    std::shared_ptr<IPipelineStep> step() const;
    QString operatorId() const;
    QString operatorDisplayName() const;

signals:
    void previewRequested();
    void addToPipelineRequested();

private:
    Ui::OperatorParameterDialog* ui = nullptr;
    ParameterPanel* m_parameterPanel = nullptr;
    std::shared_ptr<IPipelineStep> m_step;
    QString m_operatorId;
    QString m_operatorDisplayName;
};
