#include "core/detection/YoloDetectionStep.h"

#include <QElapsedTimer>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/detection/YoloDetectionDecoder.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace {

QRectF mapBoxFromLetterbox(const QRectF& box, const LetterboxResult& letterboxMeta, const cv::Size& originalSize)
{
    if (letterboxMeta.scale <= 0.0f) {
        return box;
    }

    const float x = (box.x() - letterboxMeta.padLeft) / letterboxMeta.scale;
    const float y = (box.y() - letterboxMeta.padTop) / letterboxMeta.scale;
    const float w = box.width() / letterboxMeta.scale;
    const float h = box.height() / letterboxMeta.scale;

    return QRectF(
        qBound(0.0, static_cast<double>(x), static_cast<double>(originalSize.width - 1)),
        qBound(0.0, static_cast<double>(y), static_cast<double>(originalSize.height - 1)),
        qMax(1.0, static_cast<double>(w)),
        qMax(1.0, static_cast<double>(h))
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
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
            {"letterbox", "Letterbox", "Input", StepParameterType::Boolean, m_descriptor.input.letterbox, {}, {}, {}, {}, {}, {}},
            {"color_order", "Color Order", "Input", StepParameterType::Choice, m_descriptor.input.colorOrder, {}, {}, {}, {}, {}, {
                {"RGB", "RGB"},
                {"BGR", "BGR"},
            }},
            {"confidence_threshold", "Confidence Threshold", "Detection", StepParameterType::Double, m_descriptor.postprocess.confidenceThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
            {"nms_iou_threshold", "NMS IoU Threshold", "Detection", StepParameterType::Double, m_descriptor.postprocess.nmsIouThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap YoloDetectionStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
        {"letterbox", m_descriptor.input.letterbox},
        {"color_order", m_descriptor.input.colorOrder},
        {"confidence_threshold", m_descriptor.postprocess.confidenceThreshold},
        {"nms_iou_threshold", m_descriptor.postprocess.nmsIouThreshold},
    };
}

void YoloDetectionStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
    if (values.contains("letterbox")) {
        m_descriptor.input.letterbox = values.value("letterbox").toBool();
    }
    if (values.contains("color_order")) {
        m_descriptor.input.colorOrder = values.value("color_order").toString();
    }
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

    QElapsedTimer timer;
    timer.start();

    LetterboxResult letterboxMeta;
    const cv::Mat blob = DetectionPreprocess::buildBlob(frame.workingMat, m_descriptor.input, &letterboxMeta);
    const DnnRunResult runResult = OpenCvDnnRunner::runSingleOutputModel(m_modelPath, blob);
    if (!runResult.success || runResult.outputs.isEmpty()) {
        DiagnosticsLog::instance().error("DNN", runResult.errorMessage);
        return {false, runResult.errorMessage.isEmpty() ? QString("DNN run failed") : runResult.errorMessage};
    }

    const cv::Mat output = runResult.outputs.first();
    const int lastDim = output.size[output.dims - 1];
    const cv::Mat reshaped = output.reshape(1, static_cast<int>(output.total() / lastDim));

    const cv::Size decodeSize(m_descriptor.input.width, m_descriptor.input.height);
    const YoloDecodeResult decodeResult = YoloDetectionDecoder::decode({
        reshaped,
        m_labels,
        decodeSize.width,
        decodeSize.height,
        m_descriptor.postprocess.confidenceThreshold,
        m_descriptor.postprocess.nmsIouThreshold
    });

    if (!decodeResult.success) {
        return {false, decodeResult.errorMessage};
    }

    QVector<DetectionBox> mappedBoxes = decodeResult.boxes;
    if (m_descriptor.input.letterbox) {
        for (DetectionBox& box : mappedBoxes) {
            box.box = mapBoxFromLetterbox(box.box, letterboxMeta, frame.workingMat.size());
        }
    }

    QVariantList boxes;
    for (const DetectionBox& box : mappedBoxes) {
        boxes.append(toVariantMap(box));
    }

    frame.annotations.insert("detections", mappedBoxes.size());
    frame.annotations.insert("detection_boxes", boxes);
    frame.metrics.insert("inference_time_ms", timer.elapsed());
    frame.metrics.insert("letterbox_scale", letterboxMeta.scale);
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());
    return {};
}
