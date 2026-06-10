#include "core/media/ImageFileSource.h"

#include <utility>

#include "infra/opencv/OpenCvImageIO.h"

ImageFileSource::ImageFileSource(QString filePath)
    : m_filePath(std::move(filePath))
{
}

MediaInfo ImageFileSource::mediaInfo() const
{
    MediaInfo info;
    info.kind = MediaSourceKind::ImageFile;
    info.sourceId = m_filePath;
    info.frameCount = 1;
    return info;
}

FrameReadResult ImageFileSource::read(const FrameReadRequest&)
{
    return OpenCvImageIO::loadImageFile(m_filePath);
}
