#include "ui/panels/ParameterPanel.h"

#include <algorithm>
#include <limits>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/pipeline/IPipelineStep.h"

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedParameterName(const QString& key, const QString& displayName, const QString& languageCode)
{
    if (!isChineseLanguage(languageCode)) {
        return displayName;
    }

    if (key == "kernelSize") return QStringLiteral("\u6838\u5927\u5c0f");
    if (key == "sigmaX" || key == "sigma") return QStringLiteral("Sigma");
    if (key == "width") return QStringLiteral("\u5bbd\u5ea6");
    if (key == "height") return QStringLiteral("\u9ad8\u5ea6");
    if (key == "threshold") return QStringLiteral("\u9608\u503c");
    if (key == "maxValue") return QStringLiteral("\u6700\u5927\u503c");
    if (key == "type") return QStringLiteral("\u7c7b\u578b");
    if (key == "operation") return QStringLiteral("\u64cd\u4f5c");
    if (key == "kernelWidth") return QStringLiteral("\u6838\u5bbd");
    if (key == "kernelHeight") return QStringLiteral("\u6838\u9ad8");
    if (key == "iterations") return QStringLiteral("\u8fed\u4ee3\u6b21\u6570");
    if (key == "threshold1") return QStringLiteral("\u9608\u503c 1");
    if (key == "threshold2") return QStringLiteral("\u9608\u503c 2");
    if (key == "apertureSize") return QStringLiteral("\u5b54\u5f84\u5927\u5c0f");
    if (key == "useL2Gradient") return QStringLiteral("L2 \u68af\u5ea6");
    if (key == "enabled") return QStringLiteral("\u542f\u7528");
    if (key == "mode") return QStringLiteral("\u6a21\u5f0f");
    if (key == "secondaryImagePath") return QStringLiteral("\u7b2c\u4e8c\u5f20\u56fe\u8def\u5f84");
    if (key == "direction") return QStringLiteral("\u65b9\u5411");
    if (key == "polarMode") return QStringLiteral("\u6781\u5750\u6807\u6a21\u5f0f");
    if (key == "centerMode") return QStringLiteral("\u4e2d\u5fc3\u6a21\u5f0f");
    if (key == "centerX") return QStringLiteral("\u4e2d\u5fc3 X");
    if (key == "centerY") return QStringLiteral("\u4e2d\u5fc3 Y");
    if (key == "radiusMode") return QStringLiteral("\u534a\u5f84\u6a21\u5f0f");
    if (key == "maxRadius") return QStringLiteral("\u6700\u5927\u534a\u5f84");
    if (key == "outputWidth") return QStringLiteral("\u8f93\u51fa\u5bbd\u5ea6");
    if (key == "outputHeight") return QStringLiteral("\u8f93\u51fa\u9ad8\u5ea6");
    if (key == "interpolation") return QStringLiteral("\u63d2\u503c\u65b9\u5f0f");
    if (key == "preset") return QStringLiteral("\u9884\u8bbe\u7c7b\u578b");
    if (key == "strength") return QStringLiteral("\u5f3a\u5ea6");
    if (key == "radius") return QStringLiteral("\u534a\u5f84");
    if (key == "angleDeg") return QStringLiteral("\u65b9\u5411\u89d2\u5ea6");
    if (key == "streakLength") return QStringLiteral("\u62d6\u5f71\u957f\u5ea6");
    if (key == "intensity") return QStringLiteral("\u53d1\u5149\u5f3a\u5ea6");
    if (key == "blendAlpha") return QStringLiteral("\u6df7\u5408\u900f\u660e\u5ea6");
    return displayName;
}

QString localizedChoiceLabel(const QString& parameterKey, const QString& choiceValue, const QString& choiceLabel, const QString& languageCode)
{
    if (!isChineseLanguage(languageCode)) {
        return choiceLabel;
    }

    if (parameterKey == "type") {
        if (choiceValue == "binary") return QStringLiteral("\u4e8c\u503c");
        if (choiceValue == "binary_inv") return QStringLiteral("\u53cd\u5411\u4e8c\u503c");
        if (choiceValue == "trunc") return QStringLiteral("\u622a\u65ad");
        if (choiceValue == "tozero") return QStringLiteral("\u7f6e\u96f6");
        if (choiceValue == "tozero_inv") return QStringLiteral("\u53cd\u5411\u7f6e\u96f6");
        if (choiceValue == "otsu") return QStringLiteral("\u5927\u6d25\u6cd5");
    }

    if (parameterKey == "operation") {
        if (choiceValue == "erode") return QStringLiteral("\u8150\u8680");
        if (choiceValue == "dilate") return QStringLiteral("\u81a8\u80c0");
        if (choiceValue == "open") return QStringLiteral("\u5f00\u8fd0\u7b97");
        if (choiceValue == "close") return QStringLiteral("\u95ed\u8fd0\u7b97");
    }

    if (parameterKey == "mode") {
        if (choiceValue == "binary") return QStringLiteral("\u4e8c\u503c");
        if (choiceValue == "otsu") return QStringLiteral("\u5927\u6d25\u6cd5");
    }

    return choiceLabel;
}

bool usesImageFilePicker(const StepParameter& parameter)
{
    return parameter.type == StepParameterType::String
        && parameter.key == "secondaryImagePath";
}

QString localizedBrowseText(const QString& languageCode)
{
    return isChineseLanguage(languageCode) ? QStringLiteral("\u6d4f\u89c8") : "Browse";
}

QString localizedImageFileDialogTitle(const QString& languageCode)
{
    return isChineseLanguage(languageCode)
        ? QStringLiteral("\u9009\u62e9\u5339\u914d\u53c2\u8003\u56fe\u7247")
        : "Choose Secondary Match Image";
}

}

ParameterPanel::ParameterPanel(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setMinimumHeight(72);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 6, 10, 6);
    rootLayout->setSpacing(4);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    rootLayout->addWidget(m_titleLabel);

    auto* formHost = new QWidget(this);
    formHost->setObjectName("parameterFormHost");
    formHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_formLayout = new QFormLayout(formHost);
    m_formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    m_formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_formLayout->setFormAlignment(Qt::AlignTop);
    m_formLayout->setHorizontalSpacing(10);
    m_formLayout->setVerticalSpacing(8);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("parameterScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidget(formHost);
    m_scrollArea->setVisible(false);
    m_scrollArea->setMinimumHeight(48);
    rootLayout->addWidget(m_scrollArea, 1);

    m_emptyStateLabel = new QLabel("Select a pipeline step to edit its parameters.", this);
    m_emptyStateLabel->setWordWrap(true);
    m_emptyStateLabel->setMaximumHeight(36);
    rootLayout->addWidget(m_emptyStateLabel);

    setLanguage("en");
}

void ParameterPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = isChineseLanguage(languageCode);
    m_titleLabel->setText(chinese ? QStringLiteral("\u53c2\u6570") : "Parameters");
    if (!m_step) {
        if (m_scrollArea) {
            m_scrollArea->setVisible(false);
        }
        m_emptyStateLabel->setText(chinese
            ? QStringLiteral("\u8bf7\u9009\u62e9\u4e00\u4e2a\u6d41\u7a0b\u6b65\u9aa4\u4ee5\u7f16\u8f91\u5176\u53c2\u6570\u3002")
            : "Select a pipeline step to edit its parameters.");
    } else {
        rebuildEditors();
    }
}

void ParameterPanel::setStep(const std::shared_ptr<IPipelineStep>& step)
{
    m_step = step;
    rebuildEditors();
}

void ParameterPanel::clearStep()
{
    m_step.reset();
    rebuildEditors();
}

void ParameterPanel::rebuildEditors()
{
    while (m_formLayout->rowCount() > 0) {
        m_formLayout->removeRow(0);
    }
    m_editorBindings.clear();

    if (!m_step) {
        if (m_scrollArea) {
            m_scrollArea->setVisible(false);
        }
        m_emptyStateLabel->setVisible(true);
        setLanguage(m_languageCode);
        return;
    }

    if (m_scrollArea) {
        m_scrollArea->setVisible(true);
    }
    m_emptyStateLabel->setVisible(false);
    const StepSchema schema = m_step->schema();
    const QVariantMap currentValues = m_step->parameterValues();
    QString currentGroup;

    for (const StepParameter& parameter : schema.parameters) {
        if (!parameter.group.isEmpty() && parameter.group != currentGroup) {
            currentGroup = parameter.group;
            auto* divider = new QFrame(this);
            divider->setObjectName("parameterDivider");
            divider->setProperty("parameterRole", QStringLiteral("divider"));
            divider->setFrameShape(QFrame::HLine);
            divider->setFrameShadow(QFrame::Plain);
            auto* groupLabel = new QLabel(currentGroup, this);
            groupLabel->setProperty("parameterRole", QStringLiteral("groupLabel"));
            QFont groupFont = groupLabel->font();
            groupFont.setBold(true);
            groupLabel->setFont(groupFont);
            m_formLayout->addRow(groupLabel, divider);
        }

        const QString labelText = localizedParameterName(parameter.key, parameter.displayName, m_languageCode);
        QWidget* editor = nullptr;

        if (parameter.type == StepParameterType::Integer) {
            auto* spinBox = new QSpinBox(this);
            spinBox->setMinimumHeight(24);
            spinBox->setMinimum(parameter.minimumValue.isValid() ? parameter.minimumValue.toInt() : std::numeric_limits<int>::min());
            spinBox->setMaximum(parameter.maximumValue.isValid() ? parameter.maximumValue.toInt() : std::numeric_limits<int>::max());
            if (parameter.stepValue.isValid()) {
                spinBox->setSingleStep(std::max(1, parameter.stepValue.toInt()));
            }
            spinBox->setValue(currentValues.value(parameter.key, parameter.defaultValue).toInt());
            if (!parameter.tooltip.isEmpty()) {
                spinBox->setToolTip(parameter.tooltip);
            }

            connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this, parameter](int value) {
                if (!m_step) {
                    return;
                }
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, value);
                emit parameterValuesChanged(values);
                updateConditionalVisibility(values);
            });

            editor = spinBox;
        } else if (parameter.type == StepParameterType::Double) {
            auto* doubleSpinBox = new QDoubleSpinBox(this);
            doubleSpinBox->setMinimumHeight(24);
            doubleSpinBox->setDecimals(3);
            doubleSpinBox->setMinimum(parameter.minimumValue.isValid() ? parameter.minimumValue.toDouble() : -1e9);
            doubleSpinBox->setMaximum(parameter.maximumValue.isValid() ? parameter.maximumValue.toDouble() : 1e9);
            if (parameter.stepValue.isValid()) {
                doubleSpinBox->setSingleStep(parameter.stepValue.toDouble());
            }
            doubleSpinBox->setValue(currentValues.value(parameter.key, parameter.defaultValue).toDouble());
            if (!parameter.tooltip.isEmpty()) {
                doubleSpinBox->setToolTip(parameter.tooltip);
            }

            connect(doubleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, parameter](double value) {
                if (!m_step) {
                    return;
                }
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, value);
                emit parameterValuesChanged(values);
                updateConditionalVisibility(values);
            });

            editor = doubleSpinBox;
        } else if (parameter.type == StepParameterType::Boolean) {
            auto* checkBox = new QCheckBox(this);
            checkBox->setMinimumHeight(24);
            checkBox->setChecked(currentValues.value(parameter.key, parameter.defaultValue).toBool());
            if (!parameter.tooltip.isEmpty()) {
                checkBox->setToolTip(parameter.tooltip);
            }

            connect(checkBox, &QCheckBox::toggled, this, [this, parameter](bool checked) {
                if (!m_step) {
                    return;
                }
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, checked);
                emit parameterValuesChanged(values);
                updateConditionalVisibility(values);
            });

            editor = checkBox;
        } else if (parameter.type == StepParameterType::Choice) {
            auto* comboBox = new QComboBox(this);
            comboBox->setMinimumHeight(24);
            for (const StepParameterChoice& choice : parameter.choices) {
                comboBox->addItem(localizedChoiceLabel(parameter.key, choice.value, choice.label, m_languageCode), choice.value);
            }

            const QString currentValue = currentValues.value(parameter.key, parameter.defaultValue).toString();
            const int selectedIndex = comboBox->findData(currentValue);
            comboBox->setCurrentIndex(selectedIndex >= 0 ? selectedIndex : 0);
            if (!parameter.tooltip.isEmpty()) {
                comboBox->setToolTip(parameter.tooltip);
            }

            connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, parameter, comboBox](int index) {
                if (!m_step || index < 0) {
                    return;
                }
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, comboBox->itemData(index));
                emit parameterValuesChanged(values);
                updateConditionalVisibility(values);
            });

            editor = comboBox;
        } else if (parameter.type == StepParameterType::String) {
            auto* lineEdit = new QLineEdit(this);
            lineEdit->setMinimumHeight(24);
            lineEdit->setText(currentValues.value(parameter.key, parameter.defaultValue).toString());
            if (!parameter.tooltip.isEmpty()) {
                lineEdit->setToolTip(parameter.tooltip);
            }

            connect(lineEdit, &QLineEdit::textChanged, this, [this, parameter](const QString& text) {
                if (!m_step) {
                    return;
                }
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, text);
                emit parameterValuesChanged(values);
                updateConditionalVisibility(values);
            });

            if (usesImageFilePicker(parameter)) {
                auto* container = new QWidget(this);
                auto* containerLayout = new QHBoxLayout(container);
                containerLayout->setContentsMargins(0, 0, 0, 0);
                containerLayout->setSpacing(6);
                containerLayout->addWidget(lineEdit, 1);

                auto* browseButton = new QPushButton(localizedBrowseText(m_languageCode), container);
                browseButton->setObjectName(QString("browseButton_%1").arg(parameter.key));
                if (!parameter.tooltip.isEmpty()) {
                    browseButton->setToolTip(parameter.tooltip);
                }
                connect(browseButton, &QPushButton::clicked, this, [this, lineEdit] {
                    const QString filePath = QFileDialog::getOpenFileName(
                        this,
                        localizedImageFileDialogTitle(m_languageCode),
                        lineEdit->text(),
                        "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)"
                    );
                    if (!filePath.isEmpty()) {
                        lineEdit->setText(filePath);
                    }
                });
                containerLayout->addWidget(browseButton);
                editor = container;
            } else {
                editor = lineEdit;
            }
        }

        if (!editor) {
            continue;
        }

        auto* rowLabel = new QLabel(labelText, this);
        rowLabel->setProperty("parameterRole", QStringLiteral("rowLabel"));
        rowLabel->setMinimumHeight(24);
        auto* rowWidget = new QWidget(this);
        auto* rowLayout = new QVBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(0);
        rowLayout->addWidget(editor);
        rowWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        if (auto* lineEdit = qobject_cast<QLineEdit*>(editor)) {
            lineEdit->setMinimumWidth(220);
        } else if (auto* spinBox = qobject_cast<QSpinBox*>(editor)) {
            spinBox->setMinimumWidth(220);
        } else if (auto* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(editor)) {
            doubleSpinBox->setMinimumWidth(220);
        } else if (auto* comboBox = qobject_cast<QComboBox*>(editor)) {
            comboBox->setMinimumWidth(220);
        } else if (auto* container = qobject_cast<QWidget*>(editor)) {
            container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            if (container->layout()) {
                container->layout()->setContentsMargins(0, 0, 0, 0);
            }
        }
        m_formLayout->addRow(rowLabel, rowWidget);
        m_editorBindings.append({parameter.key, parameter.visibleWhen, rowLabel, rowWidget});
    }

    updateConditionalVisibility(currentValues);
}

void ParameterPanel::updateConditionalVisibility(const QVariantMap& values)
{
    for (const ParameterEditorBinding& binding : m_editorBindings) {
        const bool visible = evaluateVisibleWhen(binding.visibleWhen, values);
        if (binding.labelWidget) {
            binding.labelWidget->setVisible(visible);
        }
        if (!binding.rowWidget) {
            continue;
        }
        binding.rowWidget->setVisible(visible);
    }
}

bool ParameterPanel::evaluateVisibleWhen(const QString& visibleWhen, const QVariantMap& values) const
{
    const QString expression = visibleWhen.trimmed();
    if (expression.isEmpty()) {
        return true;
    }

    const int notEqualIndex = expression.indexOf("!=");
    if (notEqualIndex > 0) {
        const QString key = expression.left(notEqualIndex).trimmed();
        const QString expectedValue = expression.mid(notEqualIndex + 2).trimmed();
        return values.value(key).toString() != expectedValue;
    }

    const int separatorIndex = expression.indexOf('=');
    if (separatorIndex <= 0) {
        return true;
    }

    const QString key = expression.left(separatorIndex).trimmed();
    const QString expectedValue = expression.mid(separatorIndex + 1).trimmed();
    return values.value(key).toString() == expectedValue;
}
