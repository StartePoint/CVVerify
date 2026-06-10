#pragma once

#include <QString>
#include <QStringList>

#include <opencv2/core.hpp>

#include "core/detection/DetectionTypes.h"

struct YoloDecodeRequest
{
    cv::Mat output;
    QStringList labels;
    int originalWidth = 0;
    int originalHeight = 0;
    float confidenceThreshold = 0.25f;
    float nmsIouThreshold = 0.45f;
};

struct YoloDecodeResult
{
    bool success = false;
    QString errorMessage;
    QVector<DetectionBox> boxes;
};

namespace YoloDetectionDecoder {

YoloDecodeResult decode(const YoloDecodeRequest& request);

}
