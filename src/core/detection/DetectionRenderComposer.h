#pragma once

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result);

}
