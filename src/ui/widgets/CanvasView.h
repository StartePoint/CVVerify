#pragma once

#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QWidget>

class QEvent;
class QLabel;
class QMouseEvent;
class QResizeEvent;

class CanvasView : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasView(QWidget* parent = nullptr);
    void setImage(const QImage& image);
    void setPlaceholderText(const QString& text);
    void setOverlayPoints(const QVector<QPointF>& points);
    void setOverlayRectangle(const QRectF& rectangle, bool visible);

signals:
    void imagePointClicked(const QPointF& imagePoint);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updatePixmap();
    bool mapLabelPointToImagePoint(const QPoint& labelPoint, QPointF* imagePoint) const;
    void updateDisplayedContent();

    QImage m_image;
    QLabel* m_label = nullptr;
    QString m_placeholderText = "Preview Canvas";
    QVector<QPointF> m_overlayPoints;
    bool m_hasOverlayRectangle = false;
    QRectF m_overlayRectangle;
};
