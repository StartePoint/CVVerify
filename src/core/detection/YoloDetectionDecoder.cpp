#include "core/detection/YoloDetectionDecoder.h"

#include <algorithm>
#include <vector>

#include <opencv2/dnn.hpp>

namespace {

cv::Mat normalizeOutput(const cv::Mat& output)
{
    if (output.empty()) {
        return {};
    }

    if (output.dims == 2) {
        return output;
    }

    if (output.dims == 3 && output.size[0] == 1) {
        return output.reshape(1, output.size[1]);
    }

    return output.reshape(1, static_cast<int>(output.total() / output.size[output.dims - 1]));
}

float clampUnit(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

}

namespace YoloDetectionDecoder {

YoloDecodeResult decode(const YoloDecodeRequest& request)
{
    YoloDecodeResult result;

    cv::Mat output = normalizeOutput(request.output);
    if (output.empty()) {
        result.errorMessage = "YOLO output tensor is empty";
        return result;
    }

    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<int> classIds;

    for (int row = 0; row < output.rows; ++row) {
        const float* data = output.ptr<float>(row);
        if (!data || output.cols <= 5) {
            continue;
        }

        const float objectness = data[4];
        int bestClassId = 0;
        float bestClassScore = data[5];

        for (int col = 6; col < output.cols; ++col) {
            if (data[col] > bestClassScore) {
                bestClassScore = data[col];
                bestClassId = col - 5;
            }
        }

        const float finalScore = clampUnit(objectness * bestClassScore);
        if (finalScore < request.confidenceThreshold) {
            continue;
        }

        const float cx = data[0] * request.originalWidth;
        const float cy = data[1] * request.originalHeight;
        const float w = data[2] * request.originalWidth;
        const float h = data[3] * request.originalHeight;

        const int left = static_cast<int>(cx - (w * 0.5f));
        const int top = static_cast<int>(cy - (h * 0.5f));

        boxes.emplace_back(left, top, static_cast<int>(w), static_cast<int>(h));
        scores.push_back(finalScore);
        classIds.push_back(bestClassId);
    }

    std::vector<int> kept;
    cv::dnn::NMSBoxes(boxes, scores, request.confidenceThreshold, request.nmsIouThreshold, kept);

    for (int index : kept) {
        DetectionBox box;
        box.classId = classIds[index];
        box.label = request.labels.value(box.classId, QString("class_%1").arg(box.classId));
        box.score = scores[index];
        box.box = QRectF(boxes[index].x, boxes[index].y, boxes[index].width, boxes[index].height);
        result.boxes.append(box);
    }

    result.success = true;
    return result;
}

}
