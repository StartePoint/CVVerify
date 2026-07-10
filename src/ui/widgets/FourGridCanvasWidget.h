#pragma once

#include <QImage>
#include <QWidget>

class CanvasView;
class QLabel;

class FourGridCanvasWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FourGridCanvasWidget(QWidget* parent = nullptr);

    CanvasView* originalCanvas() const;
    CanvasView* processedCanvas() const;
    CanvasView* overlayCanvas() const;
    CanvasView* intermediateCanvas() const;

    void setOriginalImage(const QImage& image);
    void setProcessedImage(const QImage& image);
    void setOverlayImage(const QImage& image);
    void setIntermediateImage(const QImage& image);

private:
    QLabel* m_originalTitle = nullptr;
    QLabel* m_processedTitle = nullptr;
    QLabel* m_overlayTitle = nullptr;
    QLabel* m_intermediateTitle = nullptr;
    CanvasView* m_originalCanvas = nullptr;
    CanvasView* m_processedCanvas = nullptr;
    CanvasView* m_overlayCanvas = nullptr;
    CanvasView* m_intermediateCanvas = nullptr;
};
