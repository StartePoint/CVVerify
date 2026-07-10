#pragma once

#include <QWidget>

class QLabel;
class QSlider;

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);

    void setRange(int minimum, int maximum);
    void setCurrentFrame(int frameIndex);
    int currentFrame() const;
    void updateFrameStatus(int frameIndex, int frameCount);
    void setIdleState();
    void setLanguage(const QString& languageCode);

signals:
    void frameChanged(int frameIndex);

private:
    void refreshStatusLabel();

    QLabel* m_statusLabel = nullptr;
    QSlider* m_slider = nullptr;
    QString m_languageCode = "en";
    int m_currentFrameIndex = 0;
    int m_frameCount = 0;
};
