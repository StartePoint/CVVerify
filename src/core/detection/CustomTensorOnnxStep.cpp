#include "core/detection/CustomTensorOnnxStep.h"

#include <QElapsedTimer>
#include <QJsonArray>

#include <opencv2/core.hpp>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace {

QVariantMap summarizeOutputTensor(const cv::Mat& output, int index)
{
    QVariantMap summary;
    summary.insert("index", index);
    summary.insert("elem_count", static_cast<qlonglong>(output.total()));
    summary.insert("channels", output.channels());
    summary.insert("depth", output.depth());

    QJsonArray dims;
    for (int dimensionIndex = 0; dimensionIndex < output.dims; ++dimensionIndex) {
        dims.append(output.size[dimensionIndex]);
    }
    summary.insert("dims", dims.toVariantList());

    if (output.empty()) {
        return summary;
    }

    cv::Mat flat = output;
    if (flat.depth() == CV_32F) {
        double minValue = 0.0;
        double maxValue = 0.0;
        cv::minMaxLoc(flat.reshape(1, 1), &minValue, &maxValue);
        const cv::Scalar meanValue = cv::mean(flat);
        summary.insert("min", minValue);
        summary.insert("max", maxValue);
        summary.insert("mean", meanValue[0]);
    }

    return summary;
}

}

CustomTensorOnnxStep::CustomTensorOnnxStep(
    DetectionModelDescriptor descriptor,
    QString modelPath,
    QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString CustomTensorOnnxStep::id() const
{
    return "dnn.custom_tensor";
}

QString CustomTensorOnnxStep::displayName() const
{
    return "ONNX Custom Tensor";
}

StepSchema CustomTensorOnnxStep::schema() const
{
    return {
        "dnn.custom_tensor",
        "ONNX Custom Tensor",
        {
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
        }
    };
}

QVariantMap CustomTensorOnnxStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
    };
}

void CustomTensorOnnxStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
}

StepResult CustomTensorOnnxStep::execute(FramePacket& frame, const RunContext&)
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
        return {false, dnnResult.errorMessage.isEmpty() ? "Custom tensor inference failed" : dnnResult.errorMessage};
    }

    QVariantList summaries;
    for (int index = 0; index < dnnResult.outputs.size(); ++index) {
        const QVariantMap summary = summarizeOutputTensor(dnnResult.outputs.at(index), index);
        summaries.append(summary);
        frame.tensorOutputs.insert(QString("output_%1").arg(index), summary);
    }

    frame.artifacts.insert("custom_tensor_outputs", summaries);
    frame.metrics.insert("custom_tensor_time_ms", timer.elapsed());
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());

    return {};
}
