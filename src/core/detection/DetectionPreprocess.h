#pragma once

#include <opencv2/core.hpp>

#include "core/detection/DetectionModelDescriptor.h"

struct LetterboxResult
{
    cv::Mat image;
    float scale = 1.0f;
    int padLeft = 0;
    int padTop = 0;
};

namespace DetectionPreprocess {

LetterboxResult letterbox(const cv::Mat& source, const DetectionInputConfig& config);
cv::Mat buildBlob(const cv::Mat& source, const DetectionInputConfig& config, LetterboxResult* letterboxMeta = nullptr);

}
