#include "ui/widgets/WipeComparisonWidget.h"

#include <QPainter>
#include <QResizeEvent>
#include <QSlider>
#include <QVBoxLayout>

WipeComparisonWidget::WipeComparisonWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setObjectName("wipeComparisonSlider");
    m_slider->setRange(0, 100);
    m_slider->setValue(m_wipePercent);
    layout->addWidget(m_slider, 0);

    layout->addStretch(1);
    connect(m_slider, &QSlider::valueChanged, this, &WipeComparisonWidget::handleSliderChanged);
}

void WipeComparisonWidget::setLeftImage(const QImage& image)
{
    m_leftImage = image;
    update();
}

void WipeComparisonWidget::setRightImage(const QImage& image)
{
    m_rightImage = image;
    update();
}

int WipeComparisonWidget::wipePosition() const
{
    return m_wipePercent;
}

void WipeComparisonWidget::handleSliderChanged(int value)
{
    m_wipePercent = value;
    update();
}

QRect WipeComparisonWidget::imageDrawRect() const
{
    const int sliderHeight = m_slider ? m_slider->height() + 12 : 0;
    return QRect(0, sliderHeight, width(), height() - sliderHeight);
}

void WipeComparisonWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor(24, 33, 43));

    const QRect drawRect = imageDrawRect();
    if (drawRect.isEmpty()) {
        return;
    }

    if (m_leftImage.isNull() && m_rightImage.isNull()) {
        painter.setPen(QColor(221, 230, 238));
        painter.drawText(drawRect, Qt::AlignCenter, "Wipe Comparison");
        return;
    }

    const QImage& baseImage = !m_leftImage.isNull() ? m_leftImage : m_rightImage;
    const QImage& overlayImage = !m_rightImage.isNull() ? m_rightImage : m_leftImage;
    const QPixmap basePixmap = QPixmap::fromImage(baseImage).scaled(drawRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QPixmap overlayPixmap = QPixmap::fromImage(overlayImage).scaled(drawRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    const QRect pixmapRect(
        drawRect.x() + (drawRect.width() - basePixmap.width()) / 2,
        drawRect.y() + (drawRect.height() - basePixmap.height()) / 2,
        basePixmap.width(),
        basePixmap.height());

    painter.drawPixmap(pixmapRect.topLeft(), basePixmap);

    const int splitX = pixmapRect.left() + (pixmapRect.width() * m_wipePercent) / 100;
    painter.save();
    painter.setClipRect(QRect(pixmapRect.left(), pixmapRect.top(), splitX - pixmapRect.left(), pixmapRect.height()));
    painter.drawPixmap(pixmapRect.topLeft(), overlayPixmap);
    painter.restore();

    painter.setPen(QPen(QColor(80, 200, 255), 2));
    painter.drawLine(splitX, pixmapRect.top(), splitX, pixmapRect.bottom());
}

void WipeComparisonWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}
