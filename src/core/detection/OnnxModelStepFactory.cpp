#include "core/detection/OnnxModelStepFactory.h"

#include "core/detection/ClassificationOnnxStep.h"
#include "core/detection/CustomTensorOnnxStep.h"
#include "core/detection/KeypointOnnxStep.h"
#include "core/detection/OcrOnnxStep.h"
#include "core/detection/SegmentationOnnxStep.h"
#include "core/detection/YoloDetectionStep.h"

namespace OnnxModelStepFactory {

namespace {

QString normalizedTaskType(const QString& taskType)
{
    return taskType.trimmed().toLower();
}

}

bool isDetectionTask(const QString& taskType)
{
    const QString normalized = normalizedTaskType(taskType);
    return normalized.isEmpty() || normalized == "detection" || normalized == "object_detection";
}

bool isClassificationTask(const QString& taskType)
{
    return normalizedTaskType(taskType) == "classification";
}

bool isSegmentationTask(const QString& taskType)
{
    const QString normalized = normalizedTaskType(taskType);
    return normalized == "segmentation"
        || normalized == "semantic_segmentation"
        || normalized == "instance_segmentation";
}

bool isOcrTask(const QString& taskType)
{
    return normalizedTaskType(taskType) == "ocr";
}

bool isKeypointTask(const QString& taskType)
{
    const QString normalized = normalizedTaskType(taskType);
    return normalized == "keypoint" || normalized == "keypoints";
}

bool isCustomTask(const QString& taskType)
{
    const QString normalized = normalizedTaskType(taskType);
    return normalized == "custom" || normalized == "tensor";
}

std::shared_ptr<IPipelineStep> createStep(
    const DetectionModelDescriptor& descriptor,
    const QString& modelPath,
    const QStringList& labels)
{
    if (isClassificationTask(descriptor.taskType)) {
        return std::make_shared<ClassificationOnnxStep>(descriptor, modelPath, labels);
    }

    if (isSegmentationTask(descriptor.taskType)) {
        return std::make_shared<SegmentationOnnxStep>(descriptor, modelPath, labels);
    }

    if (isOcrTask(descriptor.taskType)) {
        return std::make_shared<OcrOnnxStep>(descriptor, modelPath, labels);
    }

    if (isKeypointTask(descriptor.taskType)) {
        return std::make_shared<KeypointOnnxStep>(descriptor, modelPath, labels);
    }

    if (isCustomTask(descriptor.taskType)) {
        return std::make_shared<CustomTensorOnnxStep>(descriptor, modelPath, labels);
    }

    if (isDetectionTask(descriptor.taskType)) {
        return std::make_shared<YoloDetectionStep>(descriptor, modelPath, labels);
    }

    return std::make_shared<CustomTensorOnnxStep>(descriptor, modelPath, labels);
}

}
