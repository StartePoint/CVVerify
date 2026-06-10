#pragma once

#include <QString>

#include "core/media/MediaTypes.h"

namespace OpenCvImageIO {

FrameReadResult loadImageFile(const QString& filePath);
MediaInfo probeVideoFile(const QString& filePath);
FrameReadResult readVideoFrame(const QString& filePath, int frameIndex);

}
