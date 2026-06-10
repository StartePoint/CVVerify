#include "core/media/VideoFileSource.h"

#include <utility>

#include "infra/opencv/OpenCvImageIO.h"

VideoFileSource::VideoFileSource(QString filePath)
    : m_filePath(std::move(filePath))
    , m_info(OpenCvImageIO::probeVideoFile(m_filePath))
{
}

MediaInfo VideoFileSource::mediaInfo() const
{
    return m_info;
}

FrameReadResult VideoFileSource::read(const FrameReadRequest& request)
{
    return OpenCvImageIO::readVideoFrame(m_filePath, request.frameIndex);
}
