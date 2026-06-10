#include "ui/panels/MediaPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

}

MediaPanel::MediaPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    layout->addWidget(m_titleLabel);

    auto* sourceRow = new QHBoxLayout();
    sourceRow->setSpacing(8);

    m_openImageButton = new QPushButton("Open Image", this);
    m_openVideoButton = new QPushButton("Open Video", this);
    m_importYoloModelButton = new QPushButton("Import YOLO Model", this);
    m_exportDetectionImagesButton = new QPushButton("Export Detection Images", this);
    m_exportDetectionVideoButton = new QPushButton("Export Detection Video", this);
    m_videoFrameLabel = new QLabel("Video Frame", this);
    m_videoFrameSpinBox = new QSpinBox(this);
    m_previousVideoFrameButton = new QPushButton("Previous Frame", this);
    m_nextVideoFrameButton = new QPushButton("Next Frame", this);
    m_playPauseVideoButton = new QPushButton("Play", this);
    m_videoFrameSpinBox->setObjectName("videoFrameSpinBox");
    m_previousVideoFrameButton->setObjectName("previousVideoFrameButton");
    m_nextVideoFrameButton->setObjectName("nextVideoFrameButton");
    m_playPauseVideoButton->setObjectName("playPauseVideoButton");
    m_openImageButton->setMinimumHeight(32);
    m_openVideoButton->setMinimumHeight(32);
    m_importYoloModelButton->setMinimumHeight(32);
    m_exportDetectionImagesButton->setMinimumHeight(30);
    m_exportDetectionVideoButton->setMinimumHeight(30);
    m_previousVideoFrameButton->setMinimumHeight(30);
    m_nextVideoFrameButton->setMinimumHeight(30);
    m_playPauseVideoButton->setMinimumHeight(30);
    m_videoFrameSpinBox->setRange(0, 0);
    m_videoFrameSpinBox->setEnabled(false);
    m_previousVideoFrameButton->setEnabled(false);
    m_nextVideoFrameButton->setEnabled(false);
    m_playPauseVideoButton->setEnabled(false);

    sourceRow->addWidget(m_openImageButton);
    sourceRow->addWidget(m_openVideoButton);

    connect(m_openImageButton, &QPushButton::clicked, this, &MediaPanel::openImageRequested);
    connect(m_openVideoButton, &QPushButton::clicked, this, &MediaPanel::openVideoRequested);
    connect(m_importYoloModelButton, &QPushButton::clicked, this, &MediaPanel::importYoloModelRequested);
    connect(m_exportDetectionImagesButton, &QPushButton::clicked, this, &MediaPanel::exportDetectionImagesRequested);
    connect(m_exportDetectionVideoButton, &QPushButton::clicked, this, &MediaPanel::exportDetectionVideoRequested);
    connect(m_videoFrameSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &MediaPanel::videoFrameRequested);
    connect(m_previousVideoFrameButton, &QPushButton::clicked, this, &MediaPanel::previousVideoFrameRequested);
    connect(m_nextVideoFrameButton, &QPushButton::clicked, this, &MediaPanel::nextVideoFrameRequested);
    connect(m_playPauseVideoButton, &QPushButton::clicked, this, [this] {
        setVideoPlaybackState(!m_isVideoPlaying);
        emit videoPlaybackToggled(m_isVideoPlaying);
    });

    layout->addLayout(sourceRow);
    layout->addWidget(m_importYoloModelButton);

    m_exportLabel = new QLabel(this);
    QFont exportFont = m_exportLabel->font();
    exportFont.setBold(true);
    m_exportLabel->setFont(exportFont);
    layout->addWidget(m_exportLabel);

    auto* exportRow = new QHBoxLayout();
    exportRow->setSpacing(8);
    exportRow->addWidget(m_exportDetectionImagesButton);
    exportRow->addWidget(m_exportDetectionVideoButton);
    layout->addLayout(exportRow);

    layout->addWidget(m_videoFrameLabel);

    m_videoControlsWidget = new QWidget(this);
    auto* videoLayout = new QHBoxLayout(m_videoControlsWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(8);
    videoLayout->addWidget(m_previousVideoFrameButton);
    videoLayout->addWidget(m_playPauseVideoButton);
    videoLayout->addWidget(m_nextVideoFrameButton);
    videoLayout->addWidget(m_videoFrameSpinBox, 1);

    layout->addWidget(m_videoControlsWidget);
    layout->addStretch();

    m_videoFrameLabel->setVisible(false);
    m_videoControlsWidget->setVisible(false);
    setLanguage("en");
}

void MediaPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = isChineseLanguage(languageCode);

    m_titleLabel->setText(chinese ? QStringLiteral("媒体与模型") : "Media & Model");
    m_openImageButton->setText(chinese ? QStringLiteral("打开图片") : "Open Image");
    m_openVideoButton->setText(chinese ? QStringLiteral("打开视频") : "Open Video");
    m_importYoloModelButton->setText(chinese ? QStringLiteral("导入 YOLO 模型") : "Import YOLO Model");
    m_exportLabel->setText(chinese ? QStringLiteral("导出") : "Export");
    m_exportDetectionImagesButton->setText(chinese ? QStringLiteral("导出检测图片") : "Export Detection Images");
    m_exportDetectionVideoButton->setText(chinese ? QStringLiteral("导出检测视频") : "Export Detection Video");
    m_videoFrameLabel->setText(chinese ? QStringLiteral("视频帧") : "Video Frame");
    m_previousVideoFrameButton->setText(chinese ? QStringLiteral("上一帧") : "Previous Frame");
    m_nextVideoFrameButton->setText(chinese ? QStringLiteral("下一帧") : "Next Frame");
    setVideoPlaybackState(m_isVideoPlaying);
}

void MediaPanel::setVideoNavigationState(bool enabled, int frameCount, int currentFrame)
{
    const QSignalBlocker blocker(m_videoFrameSpinBox);
    const int maximumFrame = frameCount > 0 ? frameCount - 1 : 0;
    const int clampedFrame = currentFrame < 0 ? 0 : (currentFrame > maximumFrame ? maximumFrame : currentFrame);

    m_videoFrameLabel->setEnabled(enabled);
    m_videoFrameSpinBox->setEnabled(enabled);
    m_videoFrameSpinBox->setRange(0, maximumFrame);
    m_videoFrameSpinBox->setValue(clampedFrame);
    m_videoFrameLabel->setVisible(enabled);
    m_videoControlsWidget->setVisible(enabled);
    m_previousVideoFrameButton->setEnabled(enabled);
    m_nextVideoFrameButton->setEnabled(enabled);
    m_playPauseVideoButton->setEnabled(enabled);
    if (!enabled) {
        setVideoPlaybackState(false);
    }
}

void MediaPanel::setVideoPlaybackState(bool playing)
{
    m_isVideoPlaying = playing;
    const bool chinese = isChineseLanguage(m_languageCode);
    m_playPauseVideoButton->setText(m_isVideoPlaying
        ? (chinese ? QStringLiteral("暂停") : "Pause")
        : (chinese ? QStringLiteral("播放") : "Play"));
}
