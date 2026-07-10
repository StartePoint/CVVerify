#pragma once

#include <QString>

#include "core/media/IMediaSource.h"

class ImageFolderSource : public IMediaSource
{
public:
    explicit ImageFolderSource(QString folderPath);

    MediaInfo mediaInfo() const override;
    FrameReadResult read(const FrameReadRequest& request) override;
    QStringList imagePaths() const;

private:
    QString m_folderPath;
    MediaInfo m_info;
    QStringList m_imagePaths;
};
