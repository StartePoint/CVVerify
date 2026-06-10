#include "core/operators/builtin/ScharrOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForScharr(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString ScharrOperator::id() const
{
    return "builtin.edge_scharr";
}

QString ScharrOperator::displayName() const
{
    return "Scharr";
}

StepSchema ScharrOperator::schema() const
{
    return {
        "builtin.edge_scharr",
        "Scharr",
        {
            {"outputMode", "Output Mode", "梯度设置", StepParameterType::Choice, "magnitude", {}, {}, {}, {}, {}, {
                {"x", "X"},
                {"y", "Y"},
                {"magnitude", "Magnitude"},
                {"abs_sum", "Abs Sum"},
            }},
            {"scale", "Scale", "梯度设置", StepParameterType::Double, 1.0, 0.0, 20.0, 0.1, {}, {}, {}},
            {"delta", "Delta", "梯度设置", StepParameterType::Double, 0.0, -255.0, 255.0, 1.0, {}, {}, {}},
        }
    };
}

QVariantMap ScharrOperator::parameterValues() const
{
    return {
        {"outputMode", m_outputMode},
        {"scale", m_scale},
        {"delta", m_delta},
    };
}

void ScharrOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("outputMode")) {
        m_outputMode = values.value("outputMode").toString();
    }
    if (values.contains("scale")) {
        m_scale = values.value("scale").toDouble();
    }
    if (values.contains("delta")) {
        m_delta = values.value("delta").toDouble();
    }
}

StepResult ScharrOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureGrayForScharr(frame.workingMat);
    cv::Mat gradX;
    cv::Mat gradY;

    cv::Scharr(gray, gradX, CV_16S, 1, 0, m_scale, m_delta);
    cv::Scharr(gray, gradY, CV_16S, 0, 1, m_scale, m_delta);

    cv::Mat absX;
    cv::Mat absY;
    cv::convertScaleAbs(gradX, absX);
    cv::convertScaleAbs(gradY, absY);

    if (m_outputMode == "x") {
        frame.workingMat = absX;
        return {};
    }
    if (m_outputMode == "y") {
        frame.workingMat = absY;
        return {};
    }

    cv::Mat result;
    if (m_outputMode == "abs_sum") {
        cv::addWeighted(absX, 0.5, absY, 0.5, 0.0, result);
    } else {
        cv::Mat magX;
        cv::Mat magY;
        absX.convertTo(magX, CV_32F);
        absY.convertTo(magY, CV_32F);
        cv::Mat magnitude;
        cv::magnitude(magX, magY, magnitude);
        cv::convertScaleAbs(magnitude, result);
    }

    frame.workingMat = result;
    return {};
}
