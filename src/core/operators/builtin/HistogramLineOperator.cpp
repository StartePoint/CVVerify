#include "core/operators/builtin/HistogramLineOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat renderHistogramCanvas(
    const std::vector<cv::Mat>& histograms,
    const std::vector<cv::Scalar>& colors,
    int bins,
    bool normalize,
    bool logScale,
    const cv::Size& canvasSize,
    bool barMode)
{
    cv::Mat canvas(canvasSize, CV_8UC3, cv::Scalar(248, 248, 248));
    const int left = 42;
    const int top = 20;
    const int right = 18;
    const int bottom = 36;
    const int plotWidth = canvas.cols - left - right;
    const int plotHeight = canvas.rows - top - bottom;
    const int baseY = canvas.rows - bottom;

    cv::rectangle(canvas, cv::Rect(left, top, plotWidth, plotHeight), cv::Scalar(220, 220, 220), 1);
    cv::line(canvas, cv::Point(left, baseY), cv::Point(left + plotWidth, baseY), cv::Scalar(120, 120, 120), 1);
    cv::line(canvas, cv::Point(left, top), cv::Point(left, baseY), cv::Scalar(120, 120, 120), 1);

    for (std::size_t index = 0; index < histograms.size(); ++index) {
        cv::Mat hist = histograms[index].clone();
        if (logScale) {
            for (int row = 0; row < hist.rows; ++row) {
                hist.at<float>(row) = std::log1p(hist.at<float>(row));
            }
        }
        if (normalize) {
            cv::normalize(hist, hist, 0, static_cast<double>(plotHeight - 4), cv::NORM_MINMAX);
        }

        if (barMode) {
            const double binWidth = static_cast<double>(plotWidth) / std::max(1, bins);
            for (int i = 0; i < bins; ++i) {
                const float value = hist.at<float>(i);
                const int x = left + static_cast<int>(i * binWidth);
                const int width = std::max(1, static_cast<int>(std::ceil(binWidth)));
                const int y = baseY - cvRound(value);
                cv::rectangle(canvas, cv::Rect(x, y, width, std::max(1, baseY - y)), colors[index], cv::FILLED);
            }
        } else {
            std::vector<cv::Point> points;
            points.reserve(bins);
            for (int i = 0; i < bins; ++i) {
                const float value = hist.at<float>(i);
                const int x = left + cvRound((static_cast<double>(i) / std::max(1, bins - 1)) * plotWidth);
                const int y = baseY - cvRound(value);
                points.emplace_back(x, y);
            }
            cv::polylines(canvas, points, false, colors[index], 2, cv::LINE_AA);
        }
    }

    return canvas;
}

bool buildHistograms(
    const cv::Mat& source,
    const QString& channelsMode,
    int bins,
    float rangeMin,
    float rangeMax,
    std::vector<cv::Mat>* histograms,
    std::vector<cv::Scalar>* colors)
{
    if (!histograms || !colors) {
        return false;
    }

    histograms->clear();
    colors->clear();
    const float range[] = {rangeMin, rangeMax};
    const float* histRange = {range};
    const int histogramSize[] = {bins};

    if (channelsMode == "gray") {
        cv::Mat gray;
        if (source.channels() == 1) {
            gray = source;
        } else {
            cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
        }
        cv::Mat hist;
        int channels[] = {0};
        cv::calcHist(&gray, 1, channels, cv::Mat(), hist, 1, histogramSize, &histRange, true, false);
        histograms->push_back(hist);
        colors->push_back(cv::Scalar(40, 40, 40));
        return true;
    }

    if (source.channels() != 3) {
        return false;
    }

    std::vector<cv::Mat> bgrChannels;
    cv::split(source, bgrChannels);

    const auto appendHist = [&](int channelIndex, const cv::Scalar& color) {
        cv::Mat hist;
        int channels[] = {0};
        cv::calcHist(&bgrChannels[channelIndex], 1, channels, cv::Mat(), hist, 1, histogramSize, &histRange, true, false);
        histograms->push_back(hist);
        colors->push_back(color);
    };

    if (channelsMode == "b") {
        appendHist(0, cv::Scalar(255, 120, 60));
        return true;
    }
    if (channelsMode == "g") {
        appendHist(1, cv::Scalar(80, 220, 80));
        return true;
    }
    if (channelsMode == "r") {
        appendHist(2, cv::Scalar(80, 80, 255));
        return true;
    }
    if (channelsMode == "all_rgb") {
        appendHist(0, cv::Scalar(255, 120, 60));
        appendHist(1, cv::Scalar(80, 220, 80));
        appendHist(2, cv::Scalar(80, 80, 255));
        return true;
    }

    return false;
}
}

QString HistogramLineOperator::id() const
{
    return "builtin.histogram_line";
}

QString HistogramLineOperator::displayName() const
{
    return "Histogram Line";
}

StepSchema HistogramLineOperator::schema() const
{
    return {
        "builtin.histogram_line",
        "Histogram Line",
        {
            {"channels", "Channels", "统计设置", StepParameterType::Choice, "gray", {}, {}, {}, {}, {}, {
                {"gray", "Gray"},
                {"b", "Blue"},
                {"g", "Green"},
                {"r", "Red"},
                {"all_rgb", "All RGB"},
            }},
            {"bins", "Bins", "统计设置", StepParameterType::Integer, 256, 8, 512, 1, {}, {}, {}},
            {"rangeMin", "Range Min", "统计设置", StepParameterType::Double, 0.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"rangeMax", "Range Max", "统计设置", StepParameterType::Double, 255.0, 0.0, 255.0, 1.0, {}, {}, {}},
            {"normalize", "Normalize", "图表设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
            {"yScale", "Y Scale", "图表设置", StepParameterType::Choice, "linear", {}, {}, {}, {}, {}, {
                {"linear", "Linear"},
                {"log", "Log"},
            }},
        }
    };
}

QVariantMap HistogramLineOperator::parameterValues() const
{
    return {
        {"channels", m_channels},
        {"bins", m_bins},
        {"rangeMin", m_rangeMin},
        {"rangeMax", m_rangeMax},
        {"normalize", m_normalize},
        {"yScale", m_yScale},
    };
}

void HistogramLineOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("channels")) m_channels = values.value("channels").toString();
    if (values.contains("bins")) m_bins = values.value("bins").toInt();
    if (values.contains("rangeMin")) m_rangeMin = values.value("rangeMin").toDouble();
    if (values.contains("rangeMax")) m_rangeMax = values.value("rangeMax").toDouble();
    if (values.contains("normalize")) m_normalize = values.value("normalize").toBool();
    if (values.contains("yScale")) m_yScale = values.value("yScale").toString();
}

StepResult HistogramLineOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (m_rangeMax <= m_rangeMin) {
        return {false, "Histogram range max must be greater than range min"};
    }

    std::vector<cv::Mat> histograms;
    std::vector<cv::Scalar> colors;
    if (!buildHistograms(
            frame.workingMat,
            m_channels,
            std::max(8, m_bins),
            static_cast<float>(m_rangeMin),
            static_cast<float>(m_rangeMax),
            &histograms,
            &colors)) {
        return {false, "Unsupported histogram channel mode"};
    }

    frame.workingMat = renderHistogramCanvas(
        histograms,
        colors,
        std::max(8, m_bins),
        m_normalize,
        m_yScale == "log",
        cv::Size(640, 360),
        false
    );
    frame.metrics.insert("histogram_bins", std::max(8, m_bins));
    return {};
}
