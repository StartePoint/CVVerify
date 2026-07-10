#include "core/detection/OcrOnnxStep.h"

#include <QElapsedTimer>

#include <opencv2/imgproc.hpp>

#include "core/detection/DetectionPreprocess.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/diagnostics/DiagnosticsLog.h"

namespace {

QString decodeSequence(const cv::Mat& output, const QStringList& labels, float threshold)
{
    if (output.empty() || output.dims < 2) {
        return {};
    }

    int sequenceLength = 0;
    int classCount = 0;
    const float* data = nullptr;

    if (output.dims == 2) {
        sequenceLength = output.rows;
        classCount = output.cols;
        data = output.ptr<float>(0);
    } else if (output.dims == 3) {
        const int dim1 = output.size[1];
        const int dim2 = output.size[2];
        if (dim1 <= dim2) {
            sequenceLength = dim1;
            classCount = dim2;
        } else {
            sequenceLength = dim2;
            classCount = dim1;
        }
        data = output.ptr<float>(0);
    } else if (output.dims == 4) {
        sequenceLength = output.size[2];
        classCount = output.size[3];
        data = output.ptr<float>(0, 0);
    } else {
        return {};
    }

    QString text;
    int previousClass = -1;
    for (int step = 0; step < sequenceLength; ++step) {
        int bestClass = 0;
        float bestScore = data[step * classCount];
        for (int classIndex = 1; classIndex < classCount; ++classIndex) {
            const float score = data[step * classCount + classIndex];
            if (score > bestScore) {
                bestScore = score;
                bestClass = classIndex;
            }
        }

        if (bestScore < threshold || bestClass == previousClass) {
            continue;
        }
        if (bestClass == 0) {
            previousClass = bestClass;
            continue;
        }

        const QString token = bestClass > 0 && bestClass < labels.size()
            ? labels.at(bestClass)
            : QString("{%1}").arg(bestClass);
        text += token;
        previousClass = bestClass;
    }

    return text.trimmed();
}

void drawOcrText(cv::Mat& image, const QString& text)
{
    if (image.empty() || text.isEmpty()) {
        return;
    }

    int baseline = 0;
    const cv::Size textSize = cv::getTextSize(
        text.toStdString(),
        cv::FONT_HERSHEY_SIMPLEX,
        0.6,
        1,
        &baseline);
    const int y = std::max(textSize.height + 8, image.rows - 8);
    cv::rectangle(
        image,
        cv::Rect(4, y - textSize.height - 6, std::min(image.cols - 8, textSize.width + 12), textSize.height + 10),
        cv::Scalar(0, 0, 0),
        cv::FILLED);
    cv::putText(
        image,
        text.toStdString(),
        cv::Point(8, y),
        cv::FONT_HERSHEY_SIMPLEX,
        0.6,
        cv::Scalar(0, 255, 0),
        1,
        cv::LINE_AA);
}

}

OcrOnnxStep::OcrOnnxStep(
    DetectionModelDescriptor descriptor,
    QString modelPath,
    QStringList labels)
    : m_descriptor(std::move(descriptor))
    , m_modelPath(std::move(modelPath))
    , m_labels(std::move(labels))
{
}

QString OcrOnnxStep::id() const
{
    return "dnn.ocr";
}

QString OcrOnnxStep::displayName() const
{
    return "ONNX OCR";
}

StepSchema OcrOnnxStep::schema() const
{
    return {
        "dnn.ocr",
        "ONNX OCR",
        {
            {"input_width", "Input Width", "Input", StepParameterType::Integer, m_descriptor.input.width, 32, 4096, 32, {}, {}, {}},
            {"input_height", "Input Height", "Input", StepParameterType::Integer, m_descriptor.input.height, 32, 4096, 32, {}, {}, {}},
            {"confidence", "Confidence", "Output", StepParameterType::Double, m_confidenceThreshold, 0.0, 1.0, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap OcrOnnxStep::parameterValues() const
{
    return {
        {"input_width", m_descriptor.input.width},
        {"input_height", m_descriptor.input.height},
        {"confidence", m_confidenceThreshold},
    };
}

void OcrOnnxStep::setParameterValues(const QVariantMap& values)
{
    if (values.contains("input_width")) {
        m_descriptor.input.width = values.value("input_width").toInt();
    }
    if (values.contains("input_height")) {
        m_descriptor.input.height = values.value("input_height").toInt();
    }
    if (values.contains("confidence")) {
        m_confidenceThreshold = static_cast<float>(values.value("confidence").toDouble());
    }
}

StepResult OcrOnnxStep::execute(FramePacket& frame, const RunContext&)
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
        return {false, dnnResult.errorMessage.isEmpty() ? "OCR inference failed" : dnnResult.errorMessage};
    }

    const QString recognizedText = decodeSequence(dnnResult.outputs.first(), m_labels, m_confidenceThreshold);
    frame.artifacts.insert("ocr_text", recognizedText);
    frame.annotations.insert("ocr_text", recognizedText);
    drawOcrText(frame.workingMat, recognizedText);
    frame.metrics.insert("ocr_time_ms", timer.elapsed());
    DiagnosticsLog::instance().recordStepTiming(id(), displayName(), timer.elapsed());

    return {};
}
