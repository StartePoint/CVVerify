#pragma once

#include <QImage>
#include <QWidget>

class QSlider;

class WipeComparisonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WipeComparisonWidget(QWidget* parent = nullptr);

    void setLeftImage(const QImage& image);
    void setRightImage(const QImage& image);
    int wipePosition() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void handleSliderChanged(int value);

private:
    QRect imageDrawRect() const;

    QImage m_leftImage;
    QImage m_rightImage;
    QSlider* m_slider = nullptr;
    int m_wipePercent = 50;
};
