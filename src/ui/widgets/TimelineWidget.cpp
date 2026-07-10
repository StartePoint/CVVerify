#include "ui/widgets/TimelineWidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QSlider>

#include <QSignalBlocker>

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

}

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(8);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("timelineStatusLabel");
    m_statusLabel->setMinimumWidth(120);
    m_statusLabel->setMinimumHeight(28);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setObjectName("timelineSlider");
    m_slider->setEnabled(false);
    m_slider->setMinimumHeight(24);

    layout->addWidget(m_statusLabel);
    layout->addWidget(m_slider, 1);

    connect(m_slider, &QSlider::valueChanged, this, &TimelineWidget::frameChanged);
    setMinimumHeight(34);
    setIdleState();
}

void TimelineWidget::setRange(int minimum, int maximum)
{
    m_slider->setRange(minimum, maximum);
    m_slider->setEnabled(maximum > minimum);
    if (maximum <= minimum) {
        setIdleState();
    } else {
        refreshStatusLabel();
    }
}

void TimelineWidget::setCurrentFrame(int frameIndex)
{
    m_currentFrameIndex = frameIndex;
    const QSignalBlocker blocker(m_slider);
    m_slider->setValue(frameIndex);
    refreshStatusLabel();
}

int TimelineWidget::currentFrame() const
{
    return m_slider->value();
}

void TimelineWidget::updateFrameStatus(int frameIndex, int frameCount)
{
    m_currentFrameIndex = frameIndex;
    m_frameCount = frameCount;
    const QSignalBlocker blocker(m_slider);
    m_slider->setValue(frameIndex);
    refreshStatusLabel();
}

void TimelineWidget::setIdleState()
{
    m_currentFrameIndex = 0;
    m_frameCount = 0;
    m_statusLabel->setText(isChineseLanguage(m_languageCode)
        ? QStringLiteral("\u65f6\u95f4\u8f74")
        : QStringLiteral("Timeline"));
}

void TimelineWidget::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    if (m_frameCount <= 0 || !m_slider->isEnabled()) {
        setIdleState();
    } else {
        refreshStatusLabel();
    }
}

void TimelineWidget::refreshStatusLabel()
{
    if (m_frameCount <= 0 || !m_slider->isEnabled()) {
        setIdleState();
        return;
    }

    const int displayFrame = m_currentFrameIndex + 1;
    const int displayTotal = std::max(1, m_frameCount);
    m_statusLabel->setText(isChineseLanguage(m_languageCode)
        ? QStringLiteral("\u5e27 %1 / %2").arg(displayFrame).arg(displayTotal)
        : QString("Frame %1 / %2").arg(displayFrame).arg(displayTotal));
}
