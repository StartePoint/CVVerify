#pragma once

#include "core/media/MediaTypes.h"

class IMediaSource
{
public:
    virtual ~IMediaSource() = default;

    virtual MediaInfo mediaInfo() const = 0;
    virtual FrameReadResult read(const FrameReadRequest& request) = 0;
};
