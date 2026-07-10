#pragma once

#include <opencv2/core.hpp>

#include <QVariantMap>

#include "core/detection/DetectionTypes.h"

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result);
void drawClassificationResults(cv::Mat& image, const QVariantMap& artifacts);
void applyModelArtifactsOverlay(cv::Mat& image, const DetectionFrameResult& detectionResult, const QVariantMap& artifacts);

}
