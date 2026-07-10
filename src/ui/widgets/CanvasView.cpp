#include "ui/widgets/CanvasView.h"

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWheelEvent>

CanvasView::CanvasView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_label = new QLabel(m_placeholderText, this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setMinimumSize(480, 360);
    m_label->setStyleSheet("background-color: #18212b; color: #dde6ee; border: 1px solid #2e3a46; border-radius: 10px;");
    m_label->installEventFilter(this);
    layout->addWidget(m_label);
}

void CanvasView::setImage(const QImage& image)
{
    m_image = image;
    if (image.isNull()) {
        m_hasOverlayRectangle = false;
        m_overlayPoints.clear();
        m_label->clear();
        m_label->setText(m_placeholderText);
        return;
    }

    m_label->setText(QString());
    updateDisplayedContent();
}

void CanvasView::setPlaceholderText(const QString& text)
{
    m_placeholderText = text;
    if (m_image.isNull() && m_label) {
        m_label->setText(m_placeholderText);
    }
}

void CanvasView::setOverlayPoints(const QVector<QPointF>& points)
{
    m_overlayPoints = points;
    updateDisplayedContent();
}

void CanvasView::setOverlayRectangle(const QRectF& rectangle, bool visible)
{
    m_overlayRectangle = rectangle;
    m_hasOverlayRectangle = visible;
    updateDisplayedContent();
}

void CanvasView::setZoomFactor(double factor)
{
    m_zoomFactor = qBound(0.25, factor, 4.0);
    updateDisplayedContent();
}

double CanvasView::zoomFactor() const
{
    return m_zoomFactor;
}

void CanvasView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateDisplayedContent();
}

void CanvasView::updatePixmap()
{
    if (m_image.isNull()) {
        return;
    }

    const QPixmap pixmap = QPixmap::fromImage(m_image);
    m_label->setPixmap(pixmap.scaled(
        m_label->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
}

bool CanvasView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_label && event) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF imagePoint;
            if (mapLabelPointToImagePoint(mouseEvent->pos(), &imagePoint)) {
                emit imagePointClicked(imagePoint);
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF imagePoint;
            if (mapLabelPointToImagePoint(mouseEvent->pos(), &imagePoint)) {
                emit cursorMoved(imagePoint, sampleImageColor(imagePoint));
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CanvasView::wheelEvent(QWheelEvent* event)
{
    if (!event) {
        return;
    }

    const double delta = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    setZoomFactor(m_zoomFactor * delta);
    event->accept();
}

bool CanvasView::mapLabelPointToImagePoint(const QPoint& labelPoint, QPointF* imagePoint) const
{
    if (m_image.isNull() || !m_label || !imagePoint) {
        return false;
    }

    const QSize targetSize = m_label->size();
    const QPixmap scaledPixmap = QPixmap::fromImage(m_image).scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    const int xOffset = (targetSize.width() - scaledPixmap.width()) / 2;
    const int yOffset = (targetSize.height() - scaledPixmap.height()) / 2;

    const QRect pixmapRect(xOffset, yOffset, scaledPixmap.width(), scaledPixmap.height());
    if (!pixmapRect.contains(labelPoint)) {
        return false;
    }

    const double normalizedX = static_cast<double>(labelPoint.x() - pixmapRect.left()) / std::max(1, pixmapRect.width());
    const double normalizedY = static_cast<double>(labelPoint.y() - pixmapRect.top()) / std::max(1, pixmapRect.height());

    *imagePoint = QPointF(
        normalizedX * std::max(1, m_image.width() - 1),
        normalizedY * std::max(1, m_image.height() - 1)
    );
    return true;
}

void CanvasView::updateDisplayedContent()
{
    if (m_image.isNull() || !m_label) {
        return;
    }

    QImage displayImage = m_image;
    if (!m_overlayPoints.isEmpty() || m_hasOverlayRectangle) {
        if (displayImage.format() != QImage::Format_RGB32 && displayImage.format() != QImage::Format_ARGB32 && displayImage.format() != QImage::Format_ARGB32_Premultiplied) {
            displayImage = displayImage.convertToFormat(QImage::Format_ARGB32);
        } else {
            displayImage = displayImage.copy();
        }

        QPainter painter(&displayImage);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPen pen(QColor(255, 90, 90));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(QColor(255, 90, 90, 190));

        for (int index = 0; index < m_overlayPoints.size(); ++index) {
            const QPointF point = m_overlayPoints.at(index);
            painter.drawEllipse(point, 5.0, 5.0);
            painter.drawText(point + QPointF(8.0, -8.0), QString::number(index + 1));
        }

        if (m_overlayPoints.size() > 1) {
            painter.setBrush(Qt::NoBrush);
            for (int index = 0; index < m_overlayPoints.size() - 1; ++index) {
                painter.drawLine(m_overlayPoints.at(index), m_overlayPoints.at(index + 1));
            }
        }

        if (m_hasOverlayRectangle) {
            painter.setBrush(Qt::NoBrush);
            QPen rectPen(QColor(80, 200, 255));
            rectPen.setWidth(2);
            rectPen.setStyle(Qt::DashLine);
            painter.setPen(rectPen);
            painter.drawRect(m_overlayRectangle);
        }
    }

    const QPixmap pixmap = QPixmap::fromImage(displayImage);
    const QSize targetSize(
        qMax(1, static_cast<int>(m_label->size().width() * m_zoomFactor)),
        qMax(1, static_cast<int>(m_label->size().height() * m_zoomFactor)));
    m_label->setPixmap(pixmap.scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
}

QColor CanvasView::sampleImageColor(const QPointF& imagePoint) const
{
    if (m_image.isNull()) {
        return QColor();
    }

    const int x = qBound(0, static_cast<int>(imagePoint.x()), m_image.width() - 1);
    const int y = qBound(0, static_cast<int>(imagePoint.y()), m_image.height() - 1);
    return m_image.pixelColor(x, y);
}
