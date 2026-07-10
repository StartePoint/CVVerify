#pragma once

#include <QImage>
#include <QColor>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QWidget>

class QEvent;
class QLabel;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;

class CanvasView : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasView(QWidget* parent = nullptr);
    void setImage(const QImage& image);
    void setPlaceholderText(const QString& text);
    void setOverlayPoints(const QVector<QPointF>& points);
    void setOverlayRectangle(const QRectF& rectangle, bool visible);
    void setZoomFactor(double factor);
    double zoomFactor() const;

signals:
    void imagePointClicked(const QPointF& imagePoint);
    void cursorMoved(const QPointF& imagePoint, const QColor& pixelColor);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void updatePixmap();
    bool mapLabelPointToImagePoint(const QPoint& labelPoint, QPointF* imagePoint) const;
    void updateDisplayedContent();
    QColor sampleImageColor(const QPointF& imagePoint) const;

    QImage m_image;
    QLabel* m_label = nullptr;
    QString m_placeholderText = "Preview Canvas";
    QVector<QPointF> m_overlayPoints;
    bool m_hasOverlayRectangle = false;
    QRectF m_overlayRectangle;
    double m_zoomFactor = 1.0;
};
