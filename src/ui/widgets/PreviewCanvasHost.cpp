#include "ui/widgets/PreviewCanvasHost.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "ui/widgets/CanvasView.h"
#include "ui/widgets/FourGridCanvasWidget.h"
#include "ui/widgets/WipeComparisonWidget.h"

PreviewCanvasHost::PreviewCanvasHost(QWidget* parent)
    : QWidget(parent)
{
    m_stack = new QStackedWidget(this);
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->addWidget(m_stack);

    m_dualPage = new QWidget(this);
    auto* dualLayout = new QHBoxLayout(m_dualPage);
    dualLayout->setContentsMargins(0, 0, 0, 0);
    dualLayout->setSpacing(12);

    auto* sourcePanel = new QFrame(m_dualPage);
    auto* sourceLayout = new QVBoxLayout(sourcePanel);
    sourceLayout->setContentsMargins(10, 10, 10, 10);
    auto* sourceTitle = new QLabel("Source", sourcePanel);
    m_sourceTitleLabel = sourceTitle;
    m_sourceTitleLabel->setObjectName("previewSourceTitleLabel");
    QFont titleFont = sourceTitle->font();
    titleFont.setBold(true);
    sourceTitle->setFont(titleFont);
    m_sourceCanvas = new CanvasView(sourcePanel);
    sourceLayout->addWidget(sourceTitle);
    sourceLayout->addWidget(m_sourceCanvas, 1);

    auto* resultPanel = new QFrame(m_dualPage);
    auto* resultLayout = new QVBoxLayout(resultPanel);
    resultLayout->setContentsMargins(10, 10, 10, 10);
    auto* resultTitle = new QLabel("Result", resultPanel);
    m_resultTitleLabel = resultTitle;
    m_resultTitleLabel->setObjectName("previewResultTitleLabel");
    resultTitle->setFont(titleFont);
    m_resultCanvas = new CanvasView(resultPanel);
    resultLayout->addWidget(resultTitle);
    resultLayout->addWidget(m_resultCanvas, 1);

    dualLayout->addWidget(sourcePanel, 1);
    dualLayout->addWidget(resultPanel, 1);

    m_wipeWidget = new WipeComparisonWidget(this);
    m_fourGridWidget = new FourGridCanvasWidget(this);

    m_stack->addWidget(m_dualPage);
    m_stack->addWidget(m_wipeWidget);
    m_stack->addWidget(m_fourGridWidget);
    m_stack->setCurrentWidget(m_dualPage);
}

PreviewCanvasHost::Mode PreviewCanvasHost::mode() const
{
    return m_mode;
}

void PreviewCanvasHost::setMode(Mode mode)
{
    m_mode = mode;
    switch (mode) {
    case Mode::Wipe:
        m_stack->setCurrentWidget(m_wipeWidget);
        break;
    case Mode::FourGrid:
        m_stack->setCurrentWidget(m_fourGridWidget);
        break;
    case Mode::Dual:
    default:
        m_stack->setCurrentWidget(m_dualPage);
        break;
    }
}

CanvasView* PreviewCanvasHost::sourceCanvas() const
{
    return m_sourceCanvas;
}

CanvasView* PreviewCanvasHost::resultCanvas() const
{
    return m_resultCanvas;
}

void PreviewCanvasHost::setSourceImage(const QImage& image)
{
    if (m_sourceCanvas) {
        m_sourceCanvas->setImage(image);
    }
    if (m_wipeWidget) {
        m_wipeWidget->setLeftImage(image);
    }
    if (m_fourGridWidget) {
        m_fourGridWidget->setOriginalImage(image);
    }
}

void PreviewCanvasHost::setResultImage(const QImage& image)
{
    if (m_resultCanvas) {
        m_resultCanvas->setImage(image);
    }
    if (m_wipeWidget) {
        m_wipeWidget->setRightImage(image);
    }
    if (m_fourGridWidget) {
        m_fourGridWidget->setProcessedImage(image);
    }
}

void PreviewCanvasHost::setOverlayImage(const QImage& image)
{
    if (m_fourGridWidget) {
        m_fourGridWidget->setOverlayImage(image);
    }
}

void PreviewCanvasHost::setIntermediateImage(const QImage& image)
{
    if (m_fourGridWidget) {
        m_fourGridWidget->setIntermediateImage(image);
    }
}

void PreviewCanvasHost::setLanguage(const QString& languageCode)
{
    const bool chinese = languageCode.startsWith("zh", Qt::CaseInsensitive);
    if (m_sourceTitleLabel) {
        m_sourceTitleLabel->setText(chinese
            ? QStringLiteral("\u539f\u56fe")
            : QStringLiteral("Source"));
    }
    if (m_resultTitleLabel) {
        m_resultTitleLabel->setText(chinese
            ? QStringLiteral("\u7ed3\u679c")
            : QStringLiteral("Result"));
    }
    if (m_sourceCanvas) {
        m_sourceCanvas->setPlaceholderText(chinese
            ? QStringLiteral("\u539f\u59cb\u9884\u89c8")
            : QStringLiteral("Source Preview"));
    }
    if (m_resultCanvas) {
        m_resultCanvas->setPlaceholderText(chinese
            ? QStringLiteral("\u5904\u7406\u7ed3\u679c\u9884\u89c8")
            : QStringLiteral("Processed Preview"));
    }
}
