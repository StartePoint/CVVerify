#include "ui/panels/ModelListPanel.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>

#include "app/ApplicationContext.h"
#include "core/models/ModelRegistry.h"

namespace {

QString localizedText(const QString& languageCode, const QString& english, const QString& chinese)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive) ? chinese : english;
}

}

ModelListPanel::ModelListPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("modelListPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 6, 4, 4);
    layout->setSpacing(8);

    auto* title = new QLabel("Models", this);
    m_titleLabel = title;
    m_titleLabel->setObjectName("modelListTitleLabel");
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->hide();
    layout->addWidget(title);

    m_modelList = new QListWidget(this);
    m_modelList->setObjectName("modelListWidget");
    layout->addWidget(m_modelList, 1);

    m_activateButton = new QPushButton("Set Active", this);
    m_activateButton->setObjectName("activateModelButton");
    m_editButton = new QPushButton("Edit Config", this);
    m_editButton->setObjectName("editModelButton");
    m_activateButton->setMinimumHeight(30);
    m_editButton->setMinimumHeight(30);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addWidget(m_activateButton);
    buttonRow->addWidget(m_editButton);
    layout->addLayout(buttonRow);

    connect(m_modelList, &QListWidget::currentRowChanged, this, &ModelListPanel::handleSelectionChanged);
    connect(m_modelList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        handleActivateClicked();
    });
    connect(m_activateButton, &QPushButton::clicked, this, &ModelListPanel::handleActivateClicked);
    connect(m_editButton, &QPushButton::clicked, this, &ModelListPanel::handleEditClicked);
}

void ModelListPanel::refreshModels()
{
    m_modelList->clear();
    const ModelRegistry& registry = ApplicationContext::instance().modelRegistry();
    const QString activeName = registry.activeModelName();

    for (const RegisteredModelPackage& package : registry.allPackages()) {
        QString label = package.descriptor.modelName;
        if (!package.descriptor.taskType.isEmpty()) {
            label += QString(" [%1]").arg(package.descriptor.taskType);
        }
        if (package.descriptor.modelName == activeName) {
            label += " *";
        }
        auto* item = new QListWidgetItem(label, m_modelList);
        item->setData(Qt::UserRole, package.descriptor.modelName);
    }

    handleSelectionChanged();
}

void ModelListPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    if (m_titleLabel) {
        m_titleLabel->setText(localizedText(languageCode, "Models", QStringLiteral("\u6a21\u578b")));
    }
    m_activateButton->setText(localizedText(languageCode, "Set Active", QStringLiteral("设为当前")));
    m_editButton->setText(localizedText(languageCode, "Edit Config", QStringLiteral("编辑配置")));
}

void ModelListPanel::handleSelectionChanged()
{
    const bool hasSelection = m_modelList->currentItem() != nullptr;
    m_activateButton->setEnabled(hasSelection);
    m_editButton->setEnabled(hasSelection);
}

void ModelListPanel::handleActivateClicked()
{
    auto* item = m_modelList->currentItem();
    if (!item) {
        return;
    }

    const QString modelName = item->data(Qt::UserRole).toString();
    ApplicationContext::instance().modelRegistry().setActiveModelName(modelName);
    refreshModels();
    emit modelActivated(modelName);
}

void ModelListPanel::handleEditClicked()
{
    auto* item = m_modelList->currentItem();
    if (!item) {
        return;
    }
    emit editModelRequested(item->data(Qt::UserRole).toString());
}
