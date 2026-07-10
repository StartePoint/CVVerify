#include "core/pipeline/BatchPipelineRunner.h"

#include <QDir>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>

#include "infra/opencv/OpenCvImageIO.h"
#include "infra/opencv/OpenCvQtImageConverter.h"

namespace BatchPipelineRunner {

bool runImageBatch(
    const QStringList& inputImagePaths,
    const QString& outputDir,
    const std::function<bool(FramePacket*, QString*)>& processor,
    BatchResult* result,
    QString* errorMessage)
{
    if (!processor) {
        if (errorMessage) {
            *errorMessage = "Batch processor is not configured";
        }
        return false;
    }

    QDir().mkpath(outputDir);
    BatchResult localResult;

    for (const QString& imagePath : inputImagePaths) {
        if (!QFileInfo::exists(imagePath)) {
            ++localResult.failedCount;
            localResult.failedPaths.append(imagePath);
            continue;
        }

        cv::Mat image = cv::imread(OpenCvImageIO::toOpenCvFilePath(imagePath), cv::IMREAD_COLOR);
        if (image.empty()) {
            ++localResult.failedCount;
            localResult.failedPaths.append(imagePath);
            continue;
        }

        FramePacket frame;
        frame.sourceId = imagePath;
        frame.originalMat = image;
        frame.workingMat = image.clone();

        QString stepError;
        if (!processor(&frame, &stepError)) {
            ++localResult.failedCount;
            localResult.failedPaths.append(imagePath);
            continue;
        }

        const QString stem = QFileInfo(imagePath).completeBaseName();
        const QString outputPath = QDir(outputDir).filePath(stem + "_processed.png");
        if (!cv::imwrite(OpenCvImageIO::toOpenCvFilePath(outputPath), frame.workingMat)) {
            ++localResult.failedCount;
            localResult.failedPaths.append(imagePath);
            continue;
        }

        ++localResult.processedCount;
    }

    if (result) {
        *result = localResult;
    }

    if (localResult.processedCount == 0) {
        if (errorMessage) {
            *errorMessage = "No images were processed successfully";
        }
        return false;
    }

    return true;
}

}
