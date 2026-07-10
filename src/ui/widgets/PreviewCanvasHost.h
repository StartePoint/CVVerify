#pragma once

#include <QImage>
#include <QWidget>

class CanvasView;
class FourGridCanvasWidget;
class QLabel;
class WipeComparisonWidget;
class QStackedWidget;

class PreviewCanvasHost : public QWidget
{
    Q_OBJECT

public:
    enum class Mode
    {
        Dual,
        Wipe,
        FourGrid
    };

    explicit PreviewCanvasHost(QWidget* parent = nullptr);

    Mode mode() const;
    void setMode(Mode mode);

    CanvasView* sourceCanvas() const;
    CanvasView* resultCanvas() const;

    void setSourceImage(const QImage& image);
    void setResultImage(const QImage& image);
    void setOverlayImage(const QImage& image);
    void setIntermediateImage(const QImage& image);
    void setLanguage(const QString& languageCode);

private:
    QWidget* m_dualPage = nullptr;
    WipeComparisonWidget* m_wipeWidget = nullptr;
    FourGridCanvasWidget* m_fourGridWidget = nullptr;
    QStackedWidget* m_stack = nullptr;
    CanvasView* m_sourceCanvas = nullptr;
    CanvasView* m_resultCanvas = nullptr;
    QLabel* m_sourceTitleLabel = nullptr;
    QLabel* m_resultTitleLabel = nullptr;
    Mode m_mode = Mode::Dual;
};
