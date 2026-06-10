#include "core/operators/builtin/ArtEffectOperator.h"

#include <opencv2/photo.hpp>

QString ArtEffectOperator::id() const
{
    return "builtin.art_effect";
}

QString ArtEffectOperator::displayName() const
{
    return "Art Effect";
}

StepSchema ArtEffectOperator::schema() const
{
    return {
        "builtin.art_effect",
        "Art Effect",
        {
            {"styleMode", "Style Mode", "效果设置", StepParameterType::Choice, "stylization", {}, {}, {}, {}, {}, {
                {"stylization", "Stylization"},
                {"detail_enhance", "Detail Enhance"},
                {"edge_preserve", "Edge Preserve"},
            }},
            {"sigmaS", "Sigma S", "效果设置", StepParameterType::Double, 60.0, 0.0, 200.0, 1.0, {}, {}, {}},
            {"sigmaR", "Sigma R", "效果设置", StepParameterType::Double, 0.45, 0.0, 1.0, 0.01, {}, {}, {}},
        }
    };
}

QVariantMap ArtEffectOperator::parameterValues() const
{
    return {
        {"styleMode", m_styleMode},
        {"sigmaS", m_sigmaS},
        {"sigmaR", m_sigmaR},
    };
}

void ArtEffectOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("styleMode")) {
        m_styleMode = values.value("styleMode").toString();
    }
    if (values.contains("sigmaS")) {
        m_sigmaS = values.value("sigmaS").toDouble();
    }
    if (values.contains("sigmaR")) {
        m_sigmaR = values.value("sigmaR").toDouble();
    }
}

StepResult ArtEffectOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (frame.workingMat.channels() != 3) {
        return {false, "Art effect requires a 3-channel color image"};
    }

    cv::Mat output;
    if (m_styleMode == "detail_enhance") {
        cv::detailEnhance(frame.workingMat, output, static_cast<float>(m_sigmaS), static_cast<float>(m_sigmaR));
    } else if (m_styleMode == "edge_preserve") {
        cv::edgePreservingFilter(frame.workingMat, output, 1, static_cast<float>(m_sigmaS), static_cast<float>(m_sigmaR));
    } else {
        cv::stylization(frame.workingMat, output, static_cast<float>(m_sigmaS), static_cast<float>(m_sigmaR));
    }

    frame.workingMat = output;
    return {};
}
