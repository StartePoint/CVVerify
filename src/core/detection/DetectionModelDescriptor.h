#pragma once

#include <QString>

struct DetectionInputConfig
{
    int width = 640;
    int height = 640;
    int channels = 3;
    QString colorOrder = "RGB";
    bool letterbox = true;
    bool keepRatio = true;
    float scale = 0.0039215686f;
};

struct DetectionPostprocessConfig
{
    float confidenceThreshold = 0.25f;
    float nmsIouThreshold = 0.45f;
    bool multiLabel = false;
    int maxDetections = 300;
};

struct DetectionOutputConfig
{
    QString layoutType = "yolo_detect_auto";
};

struct DetectionModelDescriptor
{
    QString modelName;
    QString taskType = "detection";
    QString frameworkFamily = "yolo";
    QString modelFormat = "onnx";
    QString labelsPath = "labels.txt";
    DetectionInputConfig input;
    DetectionPostprocessConfig postprocess;
    DetectionOutputConfig output;
};

struct DetectionModelLoadResult
{
    bool success = false;
    QString errorMessage;
    DetectionModelDescriptor descriptor;
};
