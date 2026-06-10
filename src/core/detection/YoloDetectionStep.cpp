#include "core/detection/YoloDetectionStep.h"

#include <opencv2/dnn.hpp>

#include "core/detection/OpenCvDnnRunner.h"
#include "core/detection/YoloDetectionDecoder.h"

namespace {

cv::Mat buildBlob(const DetectionModelDescriptor& descriptor, const cv::Mat& image)
{
    const bool swapRB = descriptor.input.colorOrder.compare("RGB", Qt::CaseInsensitive) == 0;
    return cv::dnn::blobFromImage(
        image,
        descriptor.input.scale,
        cv::Size(descriptor.input.width, descriptor.input.height),
        cv::Scalar(),
        swapRB,
        false
    );
}

QVariantMap toVariantMap(const DetectionBox& box)
{
    return {
        {"class_id", box.classId},
        {"label", box.label},
        {"score", box.score},
        {"x", box.box.x()},
        {"y", box.box.y()},
        {"width", box.box.width()},
        {"height", box.box.height()},
    };
}

}

YoloDetectionStep::YoloDetectionStep(DetectionModelDescriptor descriptor, QString modelPath, QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString YoloDetectionStep::id() const
{
    return "detection.yolo";
}

QString YoloDetectionStep::displayName() const
{
    return "YOLO Detection";
}

StepSchema YoloDetectionStep::schema() const
{
    return {
        "detection.yolo",
        "YOLO Detection",
        {
            {"confidence_threshold", "Confidence Threshold", "Detection", StepParameterType::Double, m_descriptor.postprocess.confidenceThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
            {"nms_iou_threshold", "NMS IoU Threshold", "Detection", StepParameterType::Double, m_descriptor.postprocess.nmsIouThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap YoloDetectionStep::parameterValues() const
{
    return {
        {"confidence_threshold", m_descriptor.postprocess.confidenceThreshold},
        {"nms_iou_threshold", m_descriptor.postprocess.nmsIouThreshold},
    };
}

void YoloDetectionStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("confidence_threshold")) {
        m_descriptor.postprocess.confidenceThreshold = values.value("confidence_threshold").toFloat();
    }
    if (values.contains("nms_iou_threshold")) {
        m_descriptor.postprocess.nmsIouThreshold = values.value("nms_iou_threshold").toFloat();
    }
}

StepResult YoloDetectionStep::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (m_modelPath.isEmpty()) {
        return {false, "Model path is empty"};
    }

    const cv::Mat blob = buildBlob(m_descriptor, frame.workingMat);
    const DnnRunResult runResult = OpenCvDnnRunner::runSingleOutputModel(m_modelPath, blob);
    if (!runResult.success || runResult.outputs.isEmpty()) {
        return {false, runResult.errorMessage.isEmpty() ? QString("DNN run failed") : runResult.errorMessage};
    }

    const cv::Mat output = runResult.outputs.first();
    const int lastDim = output.size[output.dims - 1];
    const cv::Mat reshaped = output.reshape(1, static_cast<int>(output.total() / lastDim));

    const YoloDecodeResult decodeResult = YoloDetectionDecoder::decode({
        reshaped,
        m_labels,
        frame.workingMat.cols,
        frame.workingMat.rows,
        m_descriptor.postprocess.confidenceThreshold,
        m_descriptor.postprocess.nmsIouThreshold
    });

    if (!decodeResult.success) {
        return {false, decodeResult.errorMessage};
    }

    QVariantList boxes;
    for (const DetectionBox& box : decodeResult.boxes) {
        boxes.append(toVariantMap(box));
    }

    frame.annotations.insert("detections", decodeResult.boxes.size());
    frame.annotations.insert("detection_boxes", boxes);
    return {};
}
