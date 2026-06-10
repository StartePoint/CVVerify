#include "infra/opencv/OpenCvImageIO.h"

#include <QFile>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

namespace OpenCvImageIO {

FrameReadResult loadImageFile(const QString& filePath)
{
    FrameReadResult result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = QString("Failed to open image file: %1").arg(filePath);
        return result;
    }

    const QByteArray encodedImage = file.readAll();
    if (encodedImage.isEmpty()) {
        result.errorMessage = QString("Failed to read image bytes: %1").arg(filePath);
        return result;
    }

    const std::vector<uchar> buffer(encodedImage.begin(), encodedImage.end());
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    if (image.empty()) {
        result.errorMessage = QString("Failed to load image: %1").arg(filePath);
        return result;
    }

    result.success = true;
    result.frame.sourceId = filePath;
    result.frame.originalMat = image;
    result.frame.workingMat = image.clone();
    return result;
}

MediaInfo probeVideoFile(const QString& filePath)
{
    MediaInfo info;
    info.kind = MediaSourceKind::VideoFile;
    info.sourceId = filePath;

    cv::VideoCapture capture(filePath.toStdString());
    if (!capture.isOpened()) {
        return info;
    }

    info.width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    info.height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    info.frameCount = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    info.fps = capture.get(cv::CAP_PROP_FPS);
    return info;
}

FrameReadResult readVideoFrame(const QString& filePath, int frameIndex)
{
    FrameReadResult result;
    cv::VideoCapture capture(filePath.toStdString());
    if (!capture.isOpened()) {
        result.errorMessage = QString("Failed to open video: %1").arg(filePath);
        return result;
    }

    capture.set(cv::CAP_PROP_POS_FRAMES, frameIndex);

    cv::Mat frame;
    if (!capture.read(frame) || frame.empty()) {
        result.errorMessage = QString("Failed to read frame %1 from %2").arg(frameIndex).arg(filePath);
        return result;
    }

    result.success = true;
    result.frame.frameId = frameIndex;
    result.frame.sourceId = filePath;
    result.frame.originalMat = frame;
    result.frame.workingMat = frame.clone();
    return result;
}

}
