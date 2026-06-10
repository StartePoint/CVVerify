#include "core/operators/builtin/HistogramEqualizeOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForEqualize(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString HistogramEqualizeOperator::id() const
{
    return "builtin.hist_equalize";
}

QString HistogramEqualizeOperator::displayName() const
{
    return "Histogram Equalize";
}

StepSchema HistogramEqualizeOperator::schema() const
{
    return {
        "builtin.hist_equalize",
        "Histogram Equalize",
        {
            {"mode", "Mode", "模式设置", StepParameterType::Choice, "global", {}, {}, {}, {}, {}, {
                {"global", "Global"},
                {"clahe", "CLAHE"},
            }},
            {"colorMode", "Color Mode", "模式设置", StepParameterType::Choice, "gray_or_luminance", {}, {}, {}, {}, {}, {
                {"gray_only", "Gray Only"},
                {"gray_or_luminance", "Gray Or Luminance"},
            }},
            {"clipLimit", "Clip Limit", "CLAHE 设置", StepParameterType::Double, 2.0, 0.1, 40.0, 0.1, "mode=clahe", {}, {}},
            {"tileGridWidth", "Tile Grid Width", "CLAHE 设置", StepParameterType::Integer, 8, 1, 64, 1, "mode=clahe", {}, {}},
            {"tileGridHeight", "Tile Grid Height", "CLAHE 设置", StepParameterType::Integer, 8, 1, 64, 1, "mode=clahe", {}, {}},
        }
    };
}

QVariantMap HistogramEqualizeOperator::parameterValues() const
{
    return {
        {"mode", m_mode},
        {"colorMode", m_colorMode},
        {"clipLimit", m_clipLimit},
        {"tileGridWidth", m_tileGridWidth},
        {"tileGridHeight", m_tileGridHeight},
    };
}

void HistogramEqualizeOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("mode")) {
        m_mode = values.value("mode").toString();
    }
    if (values.contains("colorMode")) {
        m_colorMode = values.value("colorMode").toString();
    }
    if (values.contains("clipLimit")) {
        m_clipLimit = values.value("clipLimit").toDouble();
    }
    if (values.contains("tileGridWidth")) {
        m_tileGridWidth = values.value("tileGridWidth").toInt();
    }
    if (values.contains("tileGridHeight")) {
        m_tileGridHeight = values.value("tileGridHeight").toInt();
    }
}

StepResult HistogramEqualizeOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (m_colorMode == "gray_or_luminance" && frame.workingMat.channels() == 3) {
        cv::Mat ycrcb;
        cv::cvtColor(frame.workingMat, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        cv::split(ycrcb, channels);

        if (m_mode == "clahe") {
            cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(m_clipLimit, cv::Size(std::max(1, m_tileGridWidth), std::max(1, m_tileGridHeight)));
            clahe->apply(channels[0], channels[0]);
        } else {
            cv::equalizeHist(channels[0], channels[0]);
        }

        cv::merge(channels, ycrcb);
        cv::cvtColor(ycrcb, frame.workingMat, cv::COLOR_YCrCb2BGR);
        return {};
    }

    cv::Mat gray = ensureGrayForEqualize(frame.workingMat);
    cv::Mat result;
    if (m_mode == "clahe") {
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(m_clipLimit, cv::Size(std::max(1, m_tileGridWidth), std::max(1, m_tileGridHeight)));
        clahe->apply(gray, result);
    } else {
        cv::equalizeHist(gray, result);
    }
    frame.workingMat = result;
    return {};
}
