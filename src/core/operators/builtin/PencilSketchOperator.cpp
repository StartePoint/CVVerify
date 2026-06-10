#include "core/operators/builtin/PencilSketchOperator.h"

#include <opencv2/photo.hpp>

QString PencilSketchOperator::id() const
{
    return "builtin.pencil_sketch";
}

QString PencilSketchOperator::displayName() const
{
    return "Pencil Sketch";
}

StepSchema PencilSketchOperator::schema() const
{
    return {
        "builtin.pencil_sketch",
        "Pencil Sketch",
        {
            {"outputMode", "Output Mode", "效果设置", StepParameterType::Choice, "gray", {}, {}, {}, {}, {}, {
                {"gray", "Gray"},
                {"color", "Color"},
            }},
            {"sigmaS", "Sigma S", "效果设置", StepParameterType::Double, 60.0, 0.0, 200.0, 1.0, {}, {}, {}},
            {"sigmaR", "Sigma R", "效果设置", StepParameterType::Double, 0.07, 0.0, 1.0, 0.01, {}, {}, {}},
            {"shadeFactor", "Shade Factor", "效果设置", StepParameterType::Double, 0.02, 0.0, 0.2, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap PencilSketchOperator::parameterValues() const
{
    return {
        {"outputMode", m_outputMode},
        {"sigmaS", m_sigmaS},
        {"sigmaR", m_sigmaR},
        {"shadeFactor", m_shadeFactor},
    };
}

void PencilSketchOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("outputMode")) {
        m_outputMode = values.value("outputMode").toString();
    }
    if (values.contains("sigmaS")) {
        m_sigmaS = values.value("sigmaS").toDouble();
    }
    if (values.contains("sigmaR")) {
        m_sigmaR = values.value("sigmaR").toDouble();
    }
    if (values.contains("shadeFactor")) {
        m_shadeFactor = values.value("shadeFactor").toDouble();
    }
}

StepResult PencilSketchOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (frame.workingMat.channels() != 3) {
        return {false, "Pencil sketch requires a 3-channel color image"};
    }

    cv::Mat graySketch;
    cv::Mat colorSketch;
    cv::pencilSketch(frame.workingMat, graySketch, colorSketch, static_cast<float>(m_sigmaS), static_cast<float>(m_sigmaR), static_cast<float>(m_shadeFactor));
    frame.workingMat = (m_outputMode == "color") ? colorSketch : graySketch;
    return {};
}
