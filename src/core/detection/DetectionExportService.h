#pragma once

#include <functional>

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"
#include "core/frame/FramePacket.h"

namespace DetectionExportService {

struct VideoExportOptions
{
    int startFrame = 0;
    int endFrame = -1;
    bool sideBySide = false;
};

struct DetectionExportContext
{
    QString pipelineSnapshotJson;
    QString modelConfigJson;
    QVariantMap frameArtifacts;
    QVariantMap tensorOutputs;
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
    const DetectionExportContext& context = {},
    const VideoExportOptions& videoOptions = {}
);
bool exportComparisonImage(
    const cv::Mat& leftImage,
    const cv::Mat& rightImage,
    const QString& outputPath,
    QString* errorMessage = nullptr
);
bool exportEnvironmentSummary(
    const QString& outputDir,
    QString* errorMessage = nullptr
);
bool exportValidationReport(
    const QString& outputDir,
    const QString& summaryText,
    QString* errorMessage = nullptr
);
bool exportTensorSummary(
    const QVariantMap& tensorOutputs,
    const QVariantMap& frameArtifacts,
    const QString& outputDir,
    const QString& fileName = QString("tensor_summary.json"),
    QString* errorMessage = nullptr
);

}
