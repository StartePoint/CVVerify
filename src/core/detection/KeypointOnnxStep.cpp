#include "core/detection/KeypointOnnxStep.h"

#include <QElapsedTimer>

#include <algorithm>

#include <opencv2/imgproc.hpp>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace {

struct KeypointCandidate
{
    int channel = 0;
    int x = 0;
    int y = 0;
    float score = 0.0f;
};

QVariantList extractKeypoints(
    const cv::Mat& output,
    float threshold,
    int maxPerChannel,
    const cv::Size& targetSize)
{
    QVariantList keypoints;
    if (output.empty() || output.dims < 3) {
        return keypoints;
    }

    int channels = 1;
    int height = 0;
    int width = 0;
    if (output.dims == 3) {
        channels = output.size[0];
        height = output.size[1];
        width = output.size[2];
    } else if (output.dims == 4) {
        channels = output.size[1];
        height = output.size[2];
        width = output.size[3];
    } else {
        return keypoints;
    }

    const float scaleX = targetSize.width > 0 ? static_cast<float>(targetSize.width) / std::max(1, width) : 1.0f;
    const float scaleY = targetSize.height > 0 ? static_cast<float>(targetSize.height) / std::max(1, height) : 1.0f;

    for (int channel = 0; channel < channels; ++channel) {
        std::vector<KeypointCandidate> candidates;
        const float* channelData = output.dims == 3
            ? output.ptr<float>(channel)
            : output.ptr<float>(0, channel);

        for (int row = 1; row < height - 1; ++row) {
            for (int col = 1; col < width - 1; ++col) {
                const float score = channelData[row * width + col];
                if (score < threshold) {
                    continue;
                }

                const float left = channelData[row * width + (col - 1)];
                const float right = channelData[row * width + (col + 1)];
                const float up = channelData[(row - 1) * width + col];
                const float down = channelData[(row + 1) * width + col];
                if (score >= left && score >= right && score >= up && score >= down) {
                    candidates.push_back(KeypointCandidate{
                        channel,
                        col,
                        row,
                        score,
                    });
                }
            }
        }

        std::sort(candidates.begin(), candidates.end(), [](const KeypointCandidate& left, const KeypointCandidate& right) {
            return left.score > right.score;
        });

        const int limit = std::min(maxPerChannel, static_cast<int>(candidates.size()));
        for (int index = 0; index < limit; ++index) {
            const KeypointCandidate& candidate = candidates[static_cast<std::size_t>(index)];
            keypoints.append(QVariantMap{
                {"channel", candidate.channel},
                {"x", candidate.x * scaleX},
                {"y", candidate.y * scaleY},
                {"score", candidate.score},
            });
        }
    }

    return keypoints;
}

void drawKeypoints(cv::Mat& image, const QVariantList& keypoints)
{
    for (const QVariant& item : keypoints) {
        const QVariantMap point = item.toMap();
        const cv::Point center(
            cvRound(point.value("x").toDouble()),
            cvRound(point.value("y").toDouble()));
        cv::circle(image, center, 4, cv::Scalar(0, 255, 255), cv::FILLED, cv::LINE_AA);
        cv::circle(image, center, 6, cv::Scalar(0, 128, 255), 1, cv::LINE_AA);
    }
}

}

KeypointOnnxStep::KeypointOnnxStep(
    DetectionModelDescriptor descriptor,
    QString modelPath,
    QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString KeypointOnnxStep::id() const
{
    return "dnn.keypoint";
}

QString KeypointOnnxStep::displayName() const
{
    return "ONNX Keypoint";
}

StepSchema KeypointOnnxStep::schema() const
{
    return {
        "dnn.keypoint",
        "ONNX Keypoint",
        {
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
            {"score_threshold", "Score Threshold", "Output", StepParameterType::Double, m_scoreThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
            {"max_per_channel", "Max Per Channel", "Output", StepParameterType::Integer, m_maxKeypointsPerChannel, 1, 32, 1, {}, {}, {}},
        }
    };
}

QVariantMap KeypointOnnxStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
        {"score_threshold", m_scoreThreshold},
        {"max_per_channel", m_maxKeypointsPerChannel},
    };
}

void KeypointOnnxStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
    if (values.contains("score_threshold")) {
        m_scoreThreshold = static_cast<float>(values.value("score_threshold").toDouble());
    }
    if (values.contains("max_per_channel")) {
        m_maxKeypointsPerChannel = values.value("max_per_channel").toInt();
    }
}

StepResult KeypointOnnxStep::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (m_modelPath.isEmpty()) {
        return {false, "Model path is empty"};
    }

    QElapsedTimer timer;
    timer.start();

    const cv::Mat blob = DetectionPreprocess::buildBlob(frame.workingMat, m_descriptor.input, nullptr);
    const DnnRunResult dnnResult = OpenCvDnnRunner::runSingleOutputModel(m_modelPath, blob);
    if (!dnnResult.success || dnnResult.outputs.isEmpty()) {
        return {false, dnnResult.errorMessage.isEmpty() ? "Keypoint inference failed" : dnnResult.errorMessage};
    }

    const QVariantList keypoints = extractKeypoints(
        dnnResult.outputs.first(),
        m_scoreThreshold,
        m_maxKeypointsPerChannel,
        frame.workingMat.size());
    frame.annotations.insert("keypoints", keypoints);
    frame.artifacts.insert("keypoints", keypoints);
    frame.metrics.insert("keypoint_count", keypoints.size());
    drawKeypoints(frame.workingMat, keypoints);
    frame.metrics.insert("keypoint_time_ms", timer.elapsed());
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());

    return {};
}
