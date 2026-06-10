#pragma once

#include <QString>

#include "core/media/IMediaSource.h"

class ImageFileSource : public IMediaSource
{
public:
    explicit ImageFileSource(QString filePath);

    MediaInfo mediaInfo() const override;
    FrameReadResult read(const FrameReadRequest& request) override;

private:
    QString m_filePath;
};
