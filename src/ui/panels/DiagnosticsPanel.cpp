#include "ui/panels/DiagnosticsPanel.h"

#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/diagnostics/DiagnosticsLog.h"

DiagnosticsPanel::DiagnosticsPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("diagnosticsPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 6, 4, 4);
    layout->setSpacing(8);

    auto* toolbar = new QHBoxLayout();
    m_clearButton = new QPushButton("Clear", this);
    m_clearButton->setObjectName("clearDiagnosticsButton");
    toolbar->addStretch();
    toolbar->addWidget(m_clearButton);
    layout->addLayout(toolbar);

    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setObjectName("diagnosticsTextEdit");
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit, 1);

    connect(m_clearButton, &QPushButton::clicked, this, [this] {
        DiagnosticsLog::instance().clear();
        refresh();
    });
}

void DiagnosticsPanel::refresh()
{
    m_textEdit->setPlainText(DiagnosticsLog::instance().detailedText());
}

void DiagnosticsPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = languageCode.startsWith("zh", Qt::CaseInsensitive);
    if (m_clearButton) {
        m_clearButton->setText(chinese ? QStringLiteral("清空") : "Clear");
    }
}
