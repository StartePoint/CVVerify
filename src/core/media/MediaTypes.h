#pragma once

#include <QString>

#include "core/frame/FramePacket.h"

enum class MediaSourceKind
{
    ImageFile,
    VideoFile,
    ImageFolder
};

struct MediaInfo
{
    MediaSourceKind kind = MediaSourceKind::ImageFile;
    QString sourceId;
    int width = 0;
    int height = 0;
    int frameCount = 0;
    double fps = 0.0;
};

struct FrameReadRequest
{
    int frameIndex = 0;
};

struct FrameReadResult
{
    bool success = false;
    QString errorMessage;
    FramePacket frame;
};
