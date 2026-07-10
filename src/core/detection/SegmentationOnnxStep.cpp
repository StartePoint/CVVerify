#include "core/detection/SegmentationOnnxStep.h"

#include <QElapsedTimer>

#include <algorithm>

#include <opencv2/imgproc.hpp>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace {

cv::Mat extractMaskFromOutput(const cv::Mat& output, const cv::Size& targetSize)
{
    cv::Mat mask;
    if (output.dims == 4) {
        const int channels = output.size[1];
        const int height = output.size[2];
        const int width = output.size[3];
        if (channels == 1) {
            mask = cv::Mat(height, width, CV_32F, const_cast<float*>(output.ptr<float>(0, 0)));
            mask = mask.clone();
        } else {
            cv::Mat logits(height, width, CV_32FC(channels), const_cast<float*>(output.ptr<float>(0, 0)));
            cv::Mat classIds(height, width, CV_8U);
            for (int row = 0; row < height; ++row) {
                const float* rowPtr = logits.ptr<float>(row);
                for (int col = 0; col < width; ++col) {
                    int bestClass = 0;
                    float bestScore = rowPtr[col * channels];
                    for (int classIndex = 1; classIndex < channels; ++classIndex) {
                        const float score = rowPtr[col * channels + classIndex];
                        if (score > bestScore) {
                            bestScore = score;
                            bestClass = classIndex;
                        }
                    }
                    classIds.at<uchar>(row, col) = static_cast<uchar>(bestClass);
                }
            }
            mask = classIds;
        }
    } else if (output.dims == 3) {
        mask = output.reshape(1, output.size[1]).clone();
    } else if (output.dims == 2) {
        mask = output.clone();
    } else {
        mask = output.reshape(1, output.rows).clone();
    }

    if (mask.type() != CV_8U) {
        cv::Mat normalized;
        cv::normalize(mask, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
        mask = normalized;
    }

    if (mask.size() != targetSize) {
        cv::Mat resized;
        cv::resize(mask, resized, targetSize, 0, 0, cv::INTER_NEAREST);
        mask = resized;
    }

    return mask;
}

cv::Mat colorizeMask(const cv::Mat& mask)
{
    cv::Mat colorMask(mask.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    for (int row = 0; row < mask.rows; ++row) {
        const uchar* maskRow = mask.ptr<uchar>(row);
        cv::Vec3b* colorRow = colorMask.ptr<cv::Vec3b>(row);
        for (int col = 0; col < mask.cols; ++col) {
            const int classId = maskRow[col];
            const int hue = (classId * 47) % 180;
            colorRow[col] = cv::Vec3b(
                static_cast<uchar>((classId * 53) % 255),
                static_cast<uchar>((classId * 97) % 255),
                static_cast<uchar>(100 + hue));
        }
    }
    return colorMask;
}

}

SegmentationOnnxStep::SegmentationOnnxStep(
    DetectionModelDescriptor descriptor,
    QString modelPath,
    QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString SegmentationOnnxStep::id() const
{
    return "dnn.segmentation";
}

QString SegmentationOnnxStep::displayName() const
{
    return "ONNX Segmentation";
}

StepSchema SegmentationOnnxStep::schema() const
{
    return {
        "dnn.segmentation",
        "ONNX Segmentation",
        {
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
            {"overlay_alpha", "Overlay Alpha", "Visualization", StepParameterType::Double, m_overlayAlpha, 0.0, 1.0, 0.05, {}, {}, {}},
        }
    };
}

QVariantMap SegmentationOnnxStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
        {"overlay_alpha", m_overlayAlpha},
    };
}

void SegmentationOnnxStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
    if (values.contains("overlay_alpha")) {
        m_overlayAlpha = static_cast<float>(values.value("overlay_alpha").toDouble());
    }
}

StepResult SegmentationOnnxStep::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (m_modelPath.isEmpty()) {
        return {false, "Model path is empty"};
    }

    QElapsedTimer timer;
    timer.start();

    const cv::Size originalSize = frame.workingMat.size();
    const cv::Mat blob = DetectionPreprocess::buildBlob(frame.workingMat, m_descriptor.input, nullptr);
    const DnnRunResult dnnResult = OpenCvDnnRunner::runSingleOutputModel(m_modelPath, blob);
    if (!dnnResult.success || dnnResult.outputs.isEmpty()) {
        return {false, dnnResult.errorMessage.isEmpty() ? "Segmentation inference failed" : dnnResult.errorMessage};
    }

    const cv::Mat mask = extractMaskFromOutput(dnnResult.outputs.first(), originalSize);
    const cv::Mat colorMask = colorizeMask(mask);

    cv::Mat baseImage = frame.workingMat;
    if (baseImage.channels() == 1) {
        cv::cvtColor(baseImage, baseImage, cv::COLOR_GRAY2BGR);
    }

    cv::Mat blended;
    cv::addWeighted(baseImage, 1.0 - m_overlayAlpha, colorMask, m_overlayAlpha, 0.0, blended);
    frame.workingMat = blended;

    int uniqueClasses = 0;
    {
        std::vector<uchar> values;
        values.assign(mask.datastart, mask.dataend);
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());
        uniqueClasses = static_cast<int>(values.size());
    }

    frame.metrics.insert("segmentation_class_count", uniqueClasses);
    frame.metrics.insert("segmentation_time_ms", timer.elapsed());
    frame.artifacts.insert("segmentation_applied", true);
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());

    return {};
}
