#include <QtTest>

#include <opencv2/core.hpp>

#include "core/detection/DetectionRenderComposer.h"

class DetectionRenderComposerTest : public QObject
{
    Q_OBJECT

private slots:
    void drawsOverlayForDetections();
};

void DetectionRenderComposerTest::drawsOverlayForDetections()
{
    cv::Mat image(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectionFrameResult result;
    DetectionBox box;
    box.classId = 0;
    box.label = "person";
    box.score = 0.88f;
    box.box = QRectF(10.0, 10.0, 20.0, 20.0);
    result.boxes.append(box);

    DetectionRenderComposer::drawDetections(image, result);

    const cv::Vec3b pixel = image.at<cv::Vec3b>(10, 10);
    QVERIFY(pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0);
}

int runDetectionRenderComposerTests(int argc, char* argv[])
{
    DetectionRenderComposerTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_detection_render_composer.moc"
