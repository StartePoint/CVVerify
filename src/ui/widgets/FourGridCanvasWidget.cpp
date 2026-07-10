#include "ui/widgets/FourGridCanvasWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "ui/widgets/CanvasView.h"

FourGridCanvasWidget::FourGridCanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(8);

    auto makeCell = [this](const QString& title, CanvasView** canvasOut) {
        auto* container = new QWidget(this);
        auto* layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        auto* titleLabel = new QLabel(title, container);
        QFont font = titleLabel->font();
        font.setBold(true);
        titleLabel->setFont(font);
        auto* canvas = new CanvasView(container);
        layout->addWidget(titleLabel);
        layout->addWidget(canvas, 1);
        *canvasOut = canvas;
        return container;
    };

    grid->addWidget(makeCell("Original", &m_originalCanvas), 0, 0);
    grid->addWidget(makeCell("Processed", &m_processedCanvas), 0, 1);
    grid->addWidget(makeCell("Overlay", &m_overlayCanvas), 1, 0);
    grid->addWidget(makeCell("Intermediate", &m_intermediateCanvas), 1, 1);

    m_originalTitle = m_originalCanvas->parentWidget()->findChild<QLabel*>();
    m_processedTitle = m_processedCanvas->parentWidget()->findChild<QLabel*>();
    m_overlayTitle = m_overlayCanvas->parentWidget()->findChild<QLabel*>();
    m_intermediateTitle = m_intermediateCanvas->parentWidget()->findChild<QLabel*>();
}

CanvasView* FourGridCanvasWidget::originalCanvas() const
{
    return m_originalCanvas;
}

CanvasView* FourGridCanvasWidget::processedCanvas() const
{
    return m_processedCanvas;
}

CanvasView* FourGridCanvasWidget::overlayCanvas() const
{
    return m_overlayCanvas;
}

CanvasView* FourGridCanvasWidget::intermediateCanvas() const
{
    return m_intermediateCanvas;
}

void FourGridCanvasWidget::setOriginalImage(const QImage& image)
{
    m_originalCanvas->setImage(image);
}

void FourGridCanvasWidget::setProcessedImage(const QImage& image)
{
    m_processedCanvas->setImage(image);
}

void FourGridCanvasWidget::setOverlayImage(const QImage& image)
{
    m_overlayCanvas->setImage(image);
}

void FourGridCanvasWidget::setIntermediateImage(const QImage& image)
{
    m_intermediateCanvas->setImage(image);
}
