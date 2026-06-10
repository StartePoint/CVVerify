#pragma once

#include <QWidget>

class QPushButton;
class QSpinBox;
class QLabel;
class QWidget;

class MediaPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPanel(QWidget* parent = nullptr);
    void setLanguage(const QString& languageCode);
    void setVideoNavigationState(bool enabled, int frameCount, int currentFrame);
    void setVideoPlaybackState(bool playing);

signals:
    void openImageRequested();
    void openVideoRequested();
    void importYoloModelRequested();
    void exportDetectionImagesRequested();
    void exportDetectionVideoRequested();
    void videoFrameRequested(int frameIndex);
    void previousVideoFrameRequested();
    void nextVideoFrameRequested();
    void videoPlaybackToggled(bool playing);

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_exportLabel = nullptr;
    QPushButton* m_openImageButton = nullptr;
    QPushButton* m_openVideoButton = nullptr;
    QPushButton* m_importYoloModelButton = nullptr;
    QPushButton* m_exportDetectionImagesButton = nullptr;
    QPushButton* m_exportDetectionVideoButton = nullptr;
    QLabel* m_videoFrameLabel = nullptr;
    QSpinBox* m_videoFrameSpinBox = nullptr;
    QPushButton* m_previousVideoFrameButton = nullptr;
    QPushButton* m_nextVideoFrameButton = nullptr;
    QPushButton* m_playPauseVideoButton = nullptr;
    QWidget* m_videoControlsWidget = nullptr;
    QString m_languageCode = "en";
    bool m_isVideoPlaying = false;
};
