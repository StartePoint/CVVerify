#include <QtTest>

#include <QImage>
#include <QTemporaryDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "core/media/ImageFileSource.h"
#include "core/media/VideoFileSource.h"

class MediaSourcesTest : public QObject
{
    Q_OBJECT

private slots:
    void imageSourceLoadsGeneratedImage();
    void imageSourceLoadsUnicodeNamedImage();
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
