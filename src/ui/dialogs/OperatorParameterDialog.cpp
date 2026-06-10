#include "ui/dialogs/OperatorParameterDialog.h"

#include <QVBoxLayout>

#include "core/pipeline/IPipelineStep.h"
#include "ui/panels/ParameterPanel.h"
#include "ui_OperatorParameterDialog.h"

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

}

OperatorParameterDialog::OperatorParameterDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::OperatorParameterDialog)
{
    ui->setupUi(this);

    auto* parameterLayout = new QVBoxLayout(ui->parameterHost);
    parameterLayout->setContentsMargins(0, 0, 0, 0);
    m_parameterPanel = new ParameterPanel(ui->parameterHost);
    parameterLayout->addWidget(m_parameterPanel);

    connect(m_parameterPanel, &ParameterPanel::parameterValuesChanged, this, [this](const QVariantMap& values) {
        if (m_step) {
            m_step->setParameterValues(values);
        }
    });
    connect(ui->applyPreviewButton, &QPushButton::clicked, this, [this] {
        emit previewRequested();
    });
    connect(ui->addToPipelineButton, &QPushButton::clicked, this, [this] {
        emit addToPipelineRequested();
    });
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::close);

    setLanguage("en");
}

OperatorParameterDialog::~OperatorParameterDialog()
{
    delete ui;
}

void OperatorParameterDialog::setLanguage(const QString& languageCode)
{
    const bool chinese = isChineseLanguage(languageCode);
    setWindowTitle(chinese ? QStringLiteral("\u7b97\u5b50\u53c2\u6570") : "Operator Parameters");
    ui->hintLabel->setText(chinese
        ? QStringLiteral("\u8c03\u6574\u53c2\u6570\u540e\uff0c\u53ef\u4ee5\u5148\u505a\u5355\u7b97\u5b50\u9884\u89c8\uff0c\u6216\u5c06\u8be5\u7b97\u5b50\u52a0\u5165\u5f53\u524d\u6d41\u7a0b\u3002")
        : "Adjust parameters, preview a single operator result, or add it to the current pipeline.");
    ui->applyPreviewButton->setText(chinese ? QStringLiteral("\u5e94\u7528\u9884\u89c8") : "Apply Preview");
    ui->addToPipelineButton->setText(chinese ? QStringLiteral("\u52a0\u5165\u6d41\u7a0b") : "Add To Pipeline");
    ui->closeButton->setText(chinese ? QStringLiteral("\u5173\u95ed") : "Close");
    if (m_parameterPanel) {
        m_parameterPanel->setLanguage(languageCode);
    }
}

void OperatorParameterDialog::setOperatorStep(const QString& operatorId, const QString& displayName, const std::shared_ptr<IPipelineStep>& step)
{
    m_operatorId = operatorId;
    m_operatorDisplayName = displayName;
    m_step = step;
    ui->operatorNameLabel->setText(displayName);
    if (m_parameterPanel) {
        m_parameterPanel->setStep(m_step);
    }
}

std::shared_ptr<IPipelineStep> OperatorParameterDialog::step() const
{
    return m_step;
}

QString OperatorParameterDialog::operatorId() const
{
    return m_operatorId;
}

QString OperatorParameterDialog::operatorDisplayName() const
{
    return m_operatorDisplayName;
}
