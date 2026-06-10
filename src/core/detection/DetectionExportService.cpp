#include "core/detection/DetectionExportService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QTextStream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "core/detection/DetectionRenderComposer.h"

namespace {

QJsonObject toJson(const DetectionFrameResult& result)
{
    QJsonObject root;
    root.insert("frame_id", static_cast<qint64>(result.frameId));
    root.insert("timestamp_ms", static_cast<qint64>(result.timestampMs));
    root.insert("source_id", result.sourceId);
    root.insert("processing_time_ms", result.runtimeMeta.value("processing_time_ms").toLongLong());

    QJsonArray boxes;
    for (const DetectionBox& box : result.boxes) {
        boxes.append(QJsonObject{
            {"class_id", box.classId},
            {"label", box.label},
            {"score", box.score},
            {"x", box.box.x()},
            {"y", box.box.y()},
            {"w", box.box.width()},
            {"h", box.box.height()}
        });
    }
    root.insert("boxes", boxes);
    return root;
}

QString resultStem(const QString& sourceId)
{
    const QFileInfo fileInfo(sourceId);
    return fileInfo.completeBaseName().isEmpty() ? QString("result") : fileInfo.completeBaseName();
}

QString csvEscape(const QString& value)
{
    QString escaped = value;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
}

QString csvLine(const DetectionFrameResult& result, const DetectionBox& box)
{
    return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11\n")
        .arg(csvEscape(QFileInfo(result.sourceId).fileName()))
        .arg(result.frameId)
        .arg(result.timestampMs)
        .arg(box.classId)
        .arg(csvEscape(box.label))
        .arg(box.score, 0, 'f', 6)
        .arg(box.box.x(), 0, 'f', 3)
        .arg(box.box.y(), 0, 'f', 3)
        .arg(box.box.width(), 0, 'f', 3)
        .arg(box.box.height(), 0, 'f', 3)
        .arg(result.runtimeMeta.value("processing_time_ms").toLongLong());
}

bool writeSnapshotFile(const QString& outputDir, const QString& fileName, const QString& contents, QString* errorMessage)
{
    if (contents.isEmpty()) {
        return true;
    }

    QFile file(QDir(outputDir).filePath(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open snapshot file: %1").arg(fileName);
        }
        return false;
    }

    if (file.write(contents.toUtf8()) < 0) {
        if (errorMessage) {
            *errorMessage = QString("Failed to write snapshot file: %1").arg(fileName);
        }
        return false;
    }

    return true;
}

bool writeSnapshots(const QString& outputDir, const DetectionExportService::DetectionExportContext& context, QString* errorMessage)
{
    if (!writeSnapshotFile(outputDir, "pipeline_snapshot.json", context.pipelineSnapshotJson, errorMessage)) {
        return false;
    }

    if (!writeSnapshotFile(outputDir, "model_config_snapshot.json", context.modelConfigJson, errorMessage)) {
        return false;
    }

    return true;
}

}

namespace DetectionExportService {

bool exportImageResult(
    const cv::Mat& image,
    const DetectionFrameResult& result,
    const QString& outputDir,
    QString* errorMessage,
    const DetectionExportContext& context)
{
    QDir dir;
    if (!dir.mkpath(outputDir)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to create export directory: %1").arg(outputDir);
        }
        return false;
    }

    const QString stem = resultStem(result.sourceId);

    cv::Mat overlay = image.clone();
    DetectionRenderComposer::drawDetections(overlay, result);

    const QString overlayPath = QDir(outputDir).filePath(stem + "_overlay.png");
    if (!cv::imwrite(overlayPath.toStdString(), overlay)) {
        if (errorMessage) {
            *errorMessage = "Failed to write overlay image";
        }
        return false;
    }

    QFile jsonFile(QDir(outputDir).filePath(stem + ".json"));
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to open JSON output file";
        }
        return false;
    }

    jsonFile.write(QJsonDocument(toJson(result)).toJson(QJsonDocument::Indented));

    if (!writeSnapshots(outputDir, context, errorMessage)) {
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool exportImageBatch(
    const QStringList& inputImagePaths,
    const QString& outputDir,
    const std::function<bool(FramePacket*, DetectionFrameResult*, QString*)>& processor,
    QString* errorMessage,
    const DetectionExportContext& context)
{
    if (!processor) {
        if (errorMessage) {
            *errorMessage = "Image batch processor callback is required";
        }
        return false;
    }

    if (inputImagePaths.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "No input images were provided";
        }
        return false;
    }

    QDir dir;
    if (!dir.mkpath(outputDir)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to create export directory: %1").arg(outputDir);
        }
        return false;
    }

    QFile csvFile(QDir(outputDir).filePath("detections.csv"));
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = "Failed to open CSV output file";
        }
        return false;
    }

    QTextStream csvStream(&csvFile);
    csvStream << "source_id,frame_id,timestamp_ms,class_id,label,score,x,y,w,h,processing_time_ms\n";

    for (const QString& inputImagePath : inputImagePaths) {
        cv::Mat image = cv::imread(inputImagePath.toStdString(), cv::IMREAD_COLOR);
        if (image.empty()) {
            if (errorMessage) {
                *errorMessage = QString("Failed to load batch image: %1").arg(inputImagePath);
            }
            return false;
        }

        FramePacket frame;
        frame.frameId = 0;
        frame.timestampMs = 0;
        frame.sourceId = inputImagePath;
        frame.originalMat = image;
        frame.workingMat = image.clone();

        DetectionFrameResult detectionResult;
        QString processErrorMessage;
        if (!processor(&frame, &detectionResult, &processErrorMessage)) {
            if (errorMessage) {
                *errorMessage = processErrorMessage.isEmpty()
                    ? QString("Image batch processing failed: %1").arg(inputImagePath)
                    : processErrorMessage;
            }
            return false;
        }

        if (detectionResult.sourceId.isEmpty()) {
            detectionResult.sourceId = frame.sourceId;
        }
        detectionResult.frameId = frame.frameId;
        detectionResult.timestampMs = frame.timestampMs;

        QString exportErrorMessage;
        if (!exportImageResult(frame.workingMat, detectionResult, outputDir, &exportErrorMessage, {})) {
            if (errorMessage) {
                *errorMessage = exportErrorMessage;
            }
            return false;
        }

        for (const DetectionBox& box : detectionResult.boxes) {
            csvStream << csvLine(detectionResult, box);
        }
    }

    if (!writeSnapshots(outputDir, context, errorMessage)) {
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }

    return true;
}

bool exportVideoResult(
    const QString& inputVideoPath,
    const QString& outputDir,
    const std::function<bool(FramePacket*, DetectionFrameResult*, QString*)>& processor,
    QString* errorMessage,
    const DetectionExportContext& context)
{
    if (!processor) {
        if (errorMessage) {
            *errorMessage = "Video processor callback is required";
        }
        return false;
    }

    QDir dir;
    if (!dir.mkpath(outputDir)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to create export directory: %1").arg(outputDir);
        }
        return false;
    }

    cv::VideoCapture capture(inputVideoPath.toStdString());
    if (!capture.isOpened()) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open video: %1").arg(inputVideoPath);
        }
        return false;
    }

    const int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    const int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    const double sourceFps = capture.get(cv::CAP_PROP_FPS);
    const double fps = sourceFps > 0.0 ? sourceFps : 25.0;
    const QString stem = resultStem(inputVideoPath);
    const QString outputVideoPath = QDir(outputDir).filePath(stem + "_overlay.avi");

    cv::VideoWriter writer;
    writer.open(
        outputVideoPath.toStdString(),
        cv::CAP_OPENCV_MJPEG,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        fps,
        cv::Size(width, height)
    );
    if (!writer.isOpened()) {
        if (errorMessage) {
            *errorMessage = QString("Failed to open output video: %1").arg(outputVideoPath);
        }
        return false;
    }

    QJsonArray frames;
    QFile csvFile(QDir(outputDir).filePath(stem + ".csv"));
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = "Failed to open CSV output file";
        }
        return false;
    }

    QTextStream csvStream(&csvFile);
    csvStream << "source_id,frame_id,timestamp_ms,class_id,label,score,x,y,w,h,processing_time_ms\n";

    int frameIndex = 0;
    qint64 totalProcessingTimeMs = 0;

    while (true) {
        cv::Mat frameMat;
        if (!capture.read(frameMat) || frameMat.empty()) {
            break;
        }

        FramePacket frame;
        frame.frameId = frameIndex;
        frame.timestampMs = static_cast<qint64>((static_cast<double>(frameIndex) * 1000.0) / fps);
        frame.sourceId = inputVideoPath;
        frame.originalMat = frameMat;
        frame.workingMat = frameMat.clone();

        DetectionFrameResult detectionResult;
        QString processErrorMessage;
        QElapsedTimer timer;
        timer.start();
        if (!processor(&frame, &detectionResult, &processErrorMessage)) {
            if (errorMessage) {
                *errorMessage = processErrorMessage.isEmpty()
                    ? QString("Video processing failed at frame %1").arg(frameIndex)
                    : processErrorMessage;
            }
            return false;
        }
        const qint64 processingTimeMs = timer.elapsed();
        totalProcessingTimeMs += processingTimeMs;

        detectionResult.frameId = frame.frameId;
        detectionResult.timestampMs = frame.timestampMs;
        if (detectionResult.sourceId.isEmpty()) {
            detectionResult.sourceId = frame.sourceId;
        }
        detectionResult.runtimeMeta.insert("processing_time_ms", processingTimeMs);

        cv::Mat overlay = frame.workingMat.empty() ? frame.originalMat.clone() : frame.workingMat.clone();
        DetectionRenderComposer::drawDetections(overlay, detectionResult);
        writer.write(overlay);

        for (const DetectionBox& box : detectionResult.boxes) {
            csvStream << csvLine(detectionResult, box);
        }

        frames.append(toJson(detectionResult));
        ++frameIndex;
    }

    QFile jsonFile(QDir(outputDir).filePath(stem + ".json"));
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to open JSON output file";
        }
        return false;
    }

    QJsonObject root;
    root.insert("source_id", inputVideoPath);
    root.insert("fps", fps);
    root.insert("frame_count", frameIndex);
    root.insert("frames", frames);
    root.insert("summary", QJsonObject{
        {"total_frames", frameIndex},
        {"total_processing_time_ms", static_cast<qint64>(totalProcessingTimeMs)},
        {"average_processing_time_ms", frameIndex > 0 ? (static_cast<double>(totalProcessingTimeMs) / static_cast<double>(frameIndex)) : 0.0}
    });
    jsonFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    if (!writeSnapshots(outputDir, context, errorMessage)) {
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }

    return true;
}

}
