#include <QtTest>

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTemporaryDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "core/detection/DetectionExportService.h"
#include "core/frame/FramePacket.h"

class DetectionExportServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void exportsOverlayImageAndJson();
    void exportsSnapshotFilesWhenContextProvided();
    void exportsBatchImagesAndCsv();
    void exportsOverlayVideoAndJson();
    void exportsTensorSummaryWithImageResult();
    void exportsSideBySideVideo();
};

void DetectionExportServiceTest::exportsOverlayImageAndJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    cv::Mat image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    result.sourceId = "sample.png";

    DetectionBox box;
    box.classId = 0;
    box.label = "person";
    box.score = 0.91f;
    box.box = QRectF(4.0, 4.0, 8.0, 8.0);
    result.boxes.append(box);

    QString errorMessage;
    QVERIFY(DetectionExportService::exportImageResult(image, result, dir.path(), &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("sample_overlay.png")));
    QVERIFY(QFileInfo::exists(dir.filePath("sample.json")));
}

void DetectionExportServiceTest::exportsSnapshotFilesWhenContextProvided()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    cv::Mat image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    result.sourceId = "sample.png";

    DetectionExportService::DetectionExportContext context;
    context.pipelineSnapshotJson = "{\"pipelineId\":\"demo.pipeline\"}";
    context.modelConfigJson = "{\"model_name\":\"demo_model\"}";

    QString errorMessage;
    QVERIFY(DetectionExportService::exportImageResult(image, result, dir.path(), &errorMessage, context));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("pipeline_snapshot.json")));
    QVERIFY(QFileInfo::exists(dir.filePath("model_config_snapshot.json")));

    QFile pipelineFile(dir.filePath("pipeline_snapshot.json"));
    QVERIFY(pipelineFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(QString::fromUtf8(pipelineFile.readAll()).contains("demo.pipeline"));

    QFile modelFile(dir.filePath("model_config_snapshot.json"));
    QVERIFY(modelFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(QString::fromUtf8(modelFile.readAll()).contains("demo_model"));
}

void DetectionExportServiceTest::exportsBatchImagesAndCsv()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    const QString firstImagePath = inputDir.filePath("first.png");
    const QString secondImagePath = inputDir.filePath("second.png");
    QVERIFY(cv::imwrite(firstImagePath.toStdString(), cv::Mat(16, 16, CV_8UC3, cv::Scalar(0, 0, 0))));
    QVERIFY(cv::imwrite(secondImagePath.toStdString(), cv::Mat(16, 16, CV_8UC3, cv::Scalar(8, 8, 8))));

    DetectionExportService::DetectionExportContext context;
    context.pipelineSnapshotJson = "{\"pipelineId\":\"batch.pipeline\"}";
    context.modelConfigJson = "{\"model_name\":\"batch_model\"}";

    QString errorMessage;
    QVERIFY(DetectionExportService::exportImageBatch(
        QStringList() << firstImagePath << secondImagePath,
        outputDir.path(),
        [](FramePacket* frame, DetectionFrameResult* result, QString* processErrorMessage) -> bool {
            if (!frame || !result) {
                if (processErrorMessage) {
                    *processErrorMessage = "Invalid image batch frame state";
                }
                return false;
            }

            result->sourceId = frame->sourceId;
            result->frameId = frame->frameId;
            result->timestampMs = frame->timestampMs;

            DetectionBox box;
            box.classId = 0;
            box.label = "person";
            box.score = 0.95f;
            box.box = QRectF(1.0, 1.0, 5.0, 5.0);
            result->boxes.append(box);

            if (processErrorMessage) {
                processErrorMessage->clear();
            }
            return true;
        },
        &errorMessage,
        context
    ));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(outputDir.filePath("first_overlay.png")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("first.json")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("second_overlay.png")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("second.json")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("detections.csv")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("pipeline_snapshot.json")));
    QVERIFY(QFileInfo::exists(outputDir.filePath("model_config_snapshot.json")));

    QFile csvFile(outputDir.filePath("detections.csv"));
    QVERIFY(csvFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString csvText = QString::fromUtf8(csvFile.readAll());
    QVERIFY(csvText.contains("source_id"));
    QVERIFY(csvText.contains("first.png"));
    QVERIFY(csvText.contains("second.png"));
}

void DetectionExportServiceTest::exportsOverlayVideoAndJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString inputVideoPath = dir.filePath("sample.avi");
    cv::VideoWriter writer;
    writer.open(
        inputVideoPath.toStdString(),
        cv::CAP_OPENCV_MJPEG,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        5.0,
        cv::Size(16, 16)
    );
    QVERIFY(writer.isOpened());
    writer.write(cv::Mat(16, 16, CV_8UC3, cv::Scalar(0, 0, 0)));
    writer.write(cv::Mat(16, 16, CV_8UC3, cv::Scalar(16, 16, 16)));
    writer.release();

    DetectionExportService::DetectionExportContext context;
    context.pipelineSnapshotJson = "{\"pipelineId\":\"video.pipeline\"}";
    context.modelConfigJson = "{\"model_name\":\"video_model\"}";

    QString errorMessage;
    QVERIFY(DetectionExportService::exportVideoResult(
        inputVideoPath,
        dir.path(),
        [](FramePacket* frame, DetectionFrameResult* result, QString* processErrorMessage) -> bool {
            if (!frame || !result) {
                if (processErrorMessage) {
                    *processErrorMessage = "Invalid frame state";
                }
                return false;
            }

            result->frameId = frame->frameId;
            result->timestampMs = frame->timestampMs;
            result->sourceId = frame->sourceId;

            DetectionBox box;
            box.classId = 0;
            box.label = "person";
            box.score = 0.92f;
            box.box = QRectF(2.0, 2.0, 6.0, 6.0);
            result->boxes.append(box);

            if (processErrorMessage) {
                processErrorMessage->clear();
            }
            return true;
        },
        &errorMessage,
        context
    ));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("sample_overlay.avi")));
    QVERIFY(QFileInfo::exists(dir.filePath("sample.json")));
    QVERIFY(QFileInfo::exists(dir.filePath("sample.csv")));
    QVERIFY(QFileInfo::exists(dir.filePath("pipeline_snapshot.json")));
    QVERIFY(QFileInfo::exists(dir.filePath("model_config_snapshot.json")));

    QFile jsonFile(dir.filePath("sample.json"));
    QVERIFY(jsonFile.open(QIODevice::ReadOnly));

    const QJsonDocument document = QJsonDocument::fromJson(jsonFile.readAll());
    QVERIFY(document.isObject());
    const QJsonObject root = document.object();
    QCOMPARE(root.value("frames").toArray().size(), 2);
    QVERIFY(root.contains("summary"));
    QVERIFY(root.value("summary").toObject().value("total_frames").toInt() == 2);
    QVERIFY(root.value("summary").toObject().contains("total_processing_time_ms"));
    QVERIFY(root.value("summary").toObject().contains("average_processing_time_ms"));

    QFile csvFile(dir.filePath("sample.csv"));
    QVERIFY(csvFile.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString csvText = QString::fromUtf8(csvFile.readAll());
    QVERIFY(csvText.contains("processing_time_ms"));
    QVERIFY(csvText.contains("frame_id"));
}

void DetectionExportServiceTest::exportsTensorSummaryWithImageResult()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    cv::Mat image(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    result.sourceId = "tensor_sample.png";

    DetectionExportService::DetectionExportContext context;
    context.tensorOutputs.insert("output_0", QVariantMap{
        {"index", 0},
        {"elem_count", static_cast<qlonglong>(128)},
        {"min", 0.1},
        {"max", 0.9},
    });
    context.frameArtifacts.insert("custom_tensor_outputs", QVariantList{
        QVariantMap{{"index", 0}, {"elem_count", static_cast<qlonglong>(128)}},
    });

    QString errorMessage;
    QVERIFY(DetectionExportService::exportImageResult(image, result, dir.path(), &errorMessage, context));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("tensor_sample_tensor_summary.json")));

    QFile summaryFile(dir.filePath("tensor_sample_tensor_summary.json"));
    QVERIFY(summaryFile.open(QIODevice::ReadOnly));
    const QJsonObject root = QJsonDocument::fromJson(summaryFile.readAll()).object();
    QVERIFY(root.contains("tensor_outputs"));
    QVERIFY(root.contains("custom_tensor_outputs"));
}

void DetectionExportServiceTest::exportsSideBySideVideo()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString inputVideoPath = dir.filePath("sample.avi");
    cv::VideoWriter writer;
    writer.open(
        inputVideoPath.toStdString(),
        cv::CAP_OPENCV_MJPEG,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        5.0,
        cv::Size(16, 16)
    );
    QVERIFY(writer.isOpened());
    writer.write(cv::Mat(16, 16, CV_8UC3, cv::Scalar(0, 0, 0)));
    writer.release();

    DetectionExportService::VideoExportOptions videoOptions;
    videoOptions.sideBySide = true;

    QString errorMessage;
    QVERIFY(DetectionExportService::exportVideoResult(
        inputVideoPath,
        dir.path(),
        [](FramePacket* frame, DetectionFrameResult* result, QString* processErrorMessage) -> bool {
            if (!frame || !result) {
                if (processErrorMessage) {
                    *processErrorMessage = "Invalid frame state";
                }
                return false;
            }

            result->frameId = frame->frameId;
            result->timestampMs = frame->timestampMs;
            result->sourceId = frame->sourceId;
            frame->workingMat = frame->originalMat.clone();
            if (processErrorMessage) {
                processErrorMessage->clear();
            }
            return true;
        },
        &errorMessage,
        {},
        videoOptions
    ));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));

    QVERIFY(QFileInfo::exists(dir.filePath("sample_side_by_side.avi")));

    cv::VideoCapture capture(dir.filePath("sample_side_by_side.avi").toStdString());
    QVERIFY(capture.isOpened());
    QCOMPARE(static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH)), 32);
}

int runDetectionExportServiceTests(int argc, char* argv[])
{
    DetectionExportServiceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_detection_export_service.moc"
