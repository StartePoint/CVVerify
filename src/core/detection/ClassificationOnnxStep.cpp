#include "core/detection/ClassificationOnnxStep.h"

#include <QElapsedTimer>

#include <algorithm>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/diagnostics/DiagnosticsLog.h"

ClassificationOnnxStep::ClassificationOnnxStep(
    DetectionModelDescriptor descriptor,
    QString modelPath,
    QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString ClassificationOnnxStep::id() const
{
    return "dnn.classification";
}

QString ClassificationOnnxStep::displayName() const
{
    return "ONNX Classification";
}

StepSchema ClassificationOnnxStep::schema() const
{
    return {
        "dnn.classification",
        "ONNX Classification",
        {
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
            {"top_k", "Top K", "Output", StepParameterType::Integer, m_topK, 1, 100, 1, {}, {}, {}},
        }
    };
}

QVariantMap ClassificationOnnxStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
        {"top_k", m_topK},
    };
}

void ClassificationOnnxStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
    if (values.contains("top_k")) {
        m_topK = values.value("top_k").toInt();
    }
}

StepResult ClassificationOnnxStep::execute(FramePacket& frame, const RunContext&)
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
        return {false, dnnResult.errorMessage.isEmpty() ? "Classification inference failed" : dnnResult.errorMessage};
    }

    const cv::Mat output = dnnResult.outputs.first();
    DiagnosticsLog::instance().recordTensorShape(
        m_descriptor.modelName,
        "input",
        QString("%1x%2x%3").arg(blob.size[0]).arg(blob.size[1]).arg(blob.size[2]),
        true);
    DiagnosticsLog::instance().recordTensorShape(
        m_descriptor.modelName,
        "output",
        QString("%1x%2x%3x%4").arg(output.size[0]).arg(output.size[1]).arg(output.size[2]).arg(output.size[3]),
        false);

    cv::Mat scores = output;
    if (scores.dims == 4) {
        scores = scores.reshape(1, scores.size[1]);
    } else if (scores.dims == 2) {
        scores = scores.reshape(1, scores.rows);
    } else {
        scores = scores.reshape(1, static_cast<int>(scores.total()));
    }

    struct RankedScore
    {
        int classId = 0;
        float score = 0.0f;
    };

    std::vector<RankedScore> ranked;
    ranked.reserve(static_cast<std::size_t>(scores.cols));
    for (int classId = 0; classId < scores.cols; ++classId) {
        ranked.push_back({classId, scores.at<float>(0, classId)});
    }

    std::sort(ranked.begin(), ranked.end(), [](const RankedScore& left, const RankedScore& right) {
        return left.score > right.score;
    });

    QVariantList topPredictions;
    const int limit = std::min(m_topK, static_cast<int>(ranked.size()));
    for (int index = 0; index < limit; ++index) {
        const RankedScore& item = ranked[static_cast<std::size_t>(index)];
        const QString label = item.classId >= 0 && item.classId < m_labels.size()
            ? m_labels.at(item.classId)
            : QString("class_%1").arg(item.classId);
        topPredictions.append(QVariantMap{
            {"class_id", item.classId},
            {"label", label},
            {"score", item.score},
        });
    }

    frame.artifacts.insert("classification_top_k", topPredictions);
    frame.metrics.insert("classification_time_ms", timer.elapsed());
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());

    return {};
}
