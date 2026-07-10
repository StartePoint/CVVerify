#pragma once

#include <string>

#include <QString>

#include "core/media/MediaTypes.h"

namespace OpenCvImageIO {

std::string toOpenCvFilePath(const QString& filePath);
FrameReadResult loadImageFile(const QString& filePath);
MediaInfo probeVideoFile(const QString& filePath);
FrameReadResult readVideoFrame(const QString& filePath, int frameIndex);

}
