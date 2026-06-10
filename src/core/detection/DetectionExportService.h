#pragma once

#include <functional>

#include <QString>
#include <QStringList>

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"
#include "core/frame/FramePacket.h"

namespace DetectionExportService {

struct DetectionExportContext
{
    QString pipelineSnapshotJson;
    QString modelConfigJson;
};

bool exportImageResult(
    const cv::Mat& image,
    const DetectionFrameResult& result,
    const QString& outputDir,
    QString* errorMessage = nullptr,
    const DetectionExportContext& context = {}
);
bool exportImageBatch(
    const QStringList& inputImagePaths,
    const QString& outputDir,
    const std::function<bool(FramePacket*, DetectionFrameResult*, QString*)>& processor,
    QString* errorMessage = nullptr,
    const DetectionExportContext& context = {}
);
bool exportVideoResult(
    const QString& inputVideoPath,
    const QString& outputDir,
    const std::function<bool(FramePacket*, DetectionFrameResult*, QString*)>& processor,
    QString* errorMessage = nullptr,
    const DetectionExportContext& context = {}
);

}
