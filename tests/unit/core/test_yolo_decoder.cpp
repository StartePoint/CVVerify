#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/YoloDetectionDecoder.h"

class YoloDecoderTest : public QObject
{
    Q_OBJECT

private slots:
    void decodesSingleDetectionRow();
};

void YoloDecoderTest::decodesSingleDetectionRow()
{
    cv::Mat output(1, 6, CV_32FC1);
    output.at<float>(0, 0) = 0.5f;
    output.at<float>(0, 1) = 0.5f;
    output.at<float>(0, 2) = 0.25f;
    output.at<float>(0, 3) = 0.25f;
    output.at<float>(0, 4) = 0.90f;
    output.at<float>(0, 5) = 0.80f;

    YoloDecodeRequest request;
    request.output = output;
    request.labels = QStringList() << "person";
    request.originalWidth = 640;
    request.originalHeight = 640;
    request.confidenceThreshold = 0.25f;
    request.nmsIouThreshold = 0.45f;

    const YoloDecodeResult result = YoloDetectionDecoder::decode(request);

    QVERIFY(result.success);
    QCOMPARE(result.boxes.size(), 1);
    QCOMPARE(result.boxes.at(0).label, QString("person"));
    QVERIFY(result.boxes.at(0).score > 0.70f);
}

int runYoloDecoderTests(int argc, char* argv[])
{
    YoloDecoderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_yolo_decoder.moc"
