#pragma once

#include <QString>

#include "core/media/IMediaSource.h"

class VideoFileSource : public IMediaSource
{
public:
    explicit VideoFileSource(QString filePath);

    MediaInfo mediaInfo() const override;
    FrameReadResult read(const FrameReadRequest& request) override;

private:
    QString m_filePath;
    MediaInfo m_info;
};
