#include "core/detection/DetectionRenderComposer.h"

#include <algorithm>

#include <QVariantList>
#include <QVariantMap>

#include <opencv2/imgproc.hpp>

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result)
{
    if (image.empty()) {
        return;
    }

    for (const DetectionBox& box : result.boxes) {
        const cv::Rect rect(
            static_cast<int>(box.box.x()),
            static_cast<int>(box.box.y()),
            static_cast<int>(box.box.width()),
            static_cast<int>(box.box.height())
        );

        cv::rectangle(image, rect, cv::Scalar(0, 255, 0), 2);

        const QString labelText = QString("%1 %2").arg(box.label).arg(box.score, 0, 'f', 2);
        cv::putText(
            image,
            labelText.toStdString(),
            cv::Point(rect.x, std::max(12, rect.y - 4)),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(0, 255, 0),
            1
        );
    }
}

void drawClassificationResults(cv::Mat& image, const QVariantMap& artifacts)
{
    if (image.empty()) {
        return;
    }

    const QVariantList topPredictions = artifacts.value("classification_top_k").toList();
    if (topPredictions.isEmpty()) {
        return;
    }

    int y = 24;
    for (const QVariant& predictionValue : topPredictions) {
        const QVariantMap prediction = predictionValue.toMap();
        const QString label = prediction.value("label").toString();
        const float score = prediction.value("score").toFloat();
        const QString line = QString("%1 %2").arg(label).arg(score, 0, 'f', 3);

        cv::putText(
            image,
            line.toStdString(),
            cv::Point(12, y),
            cv::FONT_HERSHEY_SIMPLEX,
            0.6,
            cv::Scalar(0, 255, 255),
            2
        );
        y += 24;
    }
}

void applyModelArtifactsOverlay(cv::Mat& image, const DetectionFrameResult& detectionResult, const QVariantMap& artifacts)
{
    if (!detectionResult.boxes.isEmpty()) {
        drawDetections(image, detectionResult);
    }

    if (artifacts.contains("classification_top_k")) {
        drawClassificationResults(image, artifacts);
    }

    if (artifacts.contains("ocr_text")) {
        const QString text = artifacts.value("ocr_text").toString();
        if (!text.isEmpty()) {
            cv::putText(
                image,
                text.toStdString(),
                cv::Point(12, std::max(24, image.rows - 12)),
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar(0, 255, 0),
                2,
                cv::LINE_AA);
        }
    }
}

}
