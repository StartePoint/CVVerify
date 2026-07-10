#include "core/media/ImageFolderSource.h"

#include <QDir>
#include <QFileInfo>

#include "core/media/ImageFileSource.h"

namespace {

QStringList collectImagePaths(const QString& folderPath)
{
    QStringList paths;
    const QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tif", "*.tiff", "*.webp"};
    QDir dir(folderPath);
    for (const QString& filter : filters) {
        for (const QFileInfo& info : dir.entryInfoList({filter}, QDir::Files, QDir::Name)) {
            paths.append(info.absoluteFilePath());
        }
    }
    paths.sort(Qt::CaseInsensitive);
    return paths;
}

}

ImageFolderSource::ImageFolderSource(QString folderPath)
    : m_folderPath(std::move(folderPath))
    , m_imagePaths(collectImagePaths(m_folderPath))
{
    m_info.kind = MediaSourceKind::ImageFolder;
    m_info.sourceId = m_folderPath;
    m_info.frameCount = m_imagePaths.size();
    if (!m_imagePaths.isEmpty()) {
        ImageFileSource probe(m_imagePaths.first());
        const FrameReadResult probeResult = probe.read({});
        if (probeResult.success) {
            m_info.width = probeResult.frame.originalMat.cols;
            m_info.height = probeResult.frame.originalMat.rows;
        }
    }
}

MediaInfo ImageFolderSource::mediaInfo() const
{
    return m_info;
}

FrameReadResult ImageFolderSource::read(const FrameReadRequest& request)
{
    FrameReadResult result;
    if (m_imagePaths.isEmpty()) {
        result.errorMessage = QString("Image folder is empty: %1").arg(m_folderPath);
        return result;
    }

    const int index = qBound(0, request.frameIndex, m_imagePaths.size() - 1);
    ImageFileSource source(m_imagePaths.at(index));
    result = source.read({});
    if (result.success) {
        result.frame.frameId = index;
        result.frame.sourceId = m_imagePaths.at(index);
    }
    return result;
}

QStringList ImageFolderSource::imagePaths() const
{
    return m_imagePaths;
}
