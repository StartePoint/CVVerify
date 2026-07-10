#include <QtTest>

#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "core/media/ImageFileSource.h"
#include "core/media/VideoFileSource.h"

class MediaSourcesTest : public QObject
{
    Q_OBJECT

private slots:
    void imageSourceLoadsGeneratedImage();
    void imageSourceLoadsUnicodeNamedImage();
    void videoSourceLoadsUnicodeNamedVideo();
    void videoSourceReportsErrorForMissingFile();
};

void MediaSourcesTest::imageSourceLoadsGeneratedImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString filePath = dir.filePath("sample.png");

    cv::Mat image(6, 8, CV_8UC3, cv::Scalar(10, 20, 30));
    QVERIFY(cv::imwrite(filePath.toStdString(), image));

    ImageFileSource source(filePath);
    FrameReadResult result = source.read({});

    QVERIFY(result.success);
    QCOMPARE(result.frame.originalMat.cols, 8);
    QCOMPARE(result.frame.originalMat.rows, 6);
    QVERIFY(result.frame.hasWorkingFrame());
}

void MediaSourcesTest::imageSourceLoadsUnicodeNamedImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString filePath = dir.filePath(QString::fromUtf8("微信图片_测试.png"));

    QImage image(8, 6, QImage::Format_RGB32);
    image.fill(qRgb(10, 20, 30));
    QVERIFY(image.save(filePath));

    ImageFileSource source(filePath);
    FrameReadResult result = source.read({});

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.frame.originalMat.cols, 8);
    QCOMPARE(result.frame.originalMat.rows, 6);
    QVERIFY(result.frame.hasWorkingFrame());
}

void MediaSourcesTest::videoSourceLoadsUnicodeNamedVideo()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString asciiVideoPath = dir.filePath("sample.avi");
    const QString unicodeVideoPath = dir.filePath(QString::fromUtf8("视频_测试.avi"));

    cv::VideoWriter writer;
    writer.open(
        asciiVideoPath.toStdString(),
        cv::CAP_OPENCV_MJPEG,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        5.0,
        cv::Size(16, 12)
    );
    QVERIFY2(writer.isOpened(), "Failed to create temporary AVI test file");

    for (int index = 0; index < 3; ++index) {
        writer.write(cv::Mat(12, 16, CV_8UC3, cv::Scalar(index * 20, 10, 5)));
    }
    writer.release();

    QVERIFY(QFile::copy(asciiVideoPath, unicodeVideoPath));

    VideoFileSource source(unicodeVideoPath);
    FrameReadResult result = source.read({0});

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.frame.originalMat.cols, 16);
    QCOMPARE(result.frame.originalMat.rows, 12);
    QVERIFY(result.frame.hasWorkingFrame());
}

void MediaSourcesTest::videoSourceReportsErrorForMissingFile()
{
    VideoFileSource source("missing-file.avi");
    FrameReadResult result = source.read({0});

    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
}

int runMediaSourcesTests(int argc, char* argv[])
{
    MediaSourcesTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_media_sources.moc"
