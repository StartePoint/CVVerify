#pragma once

#include <functional>

#include <QString>
#include <QStringList>

#include "core/frame/FramePacket.h"

namespace BatchPipelineRunner {

struct BatchResult
{
    int processedCount = 0;
    int failedCount = 0;
    QStringList failedPaths;
};

bool runImageBatch(
    const QStringList& inputImagePaths,
    const QString& outputDir,
    const std::function<bool(FramePacket*, QString*)>& processor,
    BatchResult* result,
    QString* errorMessage = nullptr
);

}
