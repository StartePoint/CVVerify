#include "core/operators/builtin/HoughLinesOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureHoughLineInput(const cv::Mat& source)
{
    cv::Mat gray;
    if (source.channels() == 1) {
        gray = source;
    } else {
        cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat edges;
    cv::Canny(gray, edges, 50.0, 150.0);
    return edges;
}
}

QString HoughLinesOperator::id() const
{
    return "builtin.hough_lines";
}

QString HoughLinesOperator::displayName() const
{
    return "Hough Lines";
}

StepSchema HoughLinesOperator::schema() const
{
    return {
        "builtin.hough_lines",
        "Hough Lines",
        {
            {"mode", "Mode", "检测设置", StepParameterType::Choice, "probabilistic", {}, {}, {}, {}, {}, {
                {"standard", "Standard"},
                {"probabilistic", "Probabilistic"},
            }},
            {"rho", "Rho", "检测设置", StepParameterType::Double, 1.0, 0.1, 10.0, 0.1, {}, {}, {}},
            {"thetaDeg", "Theta Deg", "检测设置", StepParameterType::Double, 1.0, 0.1, 180.0, 0.1, {}, {}, {}},
            {"threshold", "Threshold", "检测设置", StepParameterType::Integer, 80, 1, 1000, 1, {}, {}, {}},
            {"minLineLength", "Min Line Length", "检测设置", StepParameterType::Double, 30.0, 0.0, 100000.0, 1.0, "mode=probabilistic", {}, {}},
            {"maxLineGap", "Max Line Gap", "检测设置", StepParameterType::Double, 10.0, 0.0, 100000.0, 1.0, "mode=probabilistic", {}, {}},
        }
    };
}

QVariantMap HoughLinesOperator::parameterValues() const
{
    return {
        {"mode", m_mode},
        {"rho", m_rho},
        {"thetaDeg", m_thetaDeg},
        {"threshold", m_threshold},
        {"minLineLength", m_minLineLength},
        {"maxLineGap", m_maxLineGap},
    };
}

void HoughLinesOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("mode")) m_mode = values.value("mode").toString();
    if (values.contains("rho")) m_rho = values.value("rho").toDouble();
    if (values.contains("thetaDeg")) m_thetaDeg = values.value("thetaDeg").toDouble();
    if (values.contains("threshold")) m_threshold = values.value("threshold").toInt();
    if (values.contains("minLineLength")) m_minLineLength = values.value("minLineLength").toDouble();
    if (values.contains("maxLineGap")) m_maxLineGap = values.value("maxLineGap").toDouble();
}

StepResult HoughLinesOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat edges = ensureHoughLineInput(frame.workingMat);
    cv::Mat output;
    if (frame.workingMat.channels() == 1) {
        cv::cvtColor(frame.workingMat, output, cv::COLOR_GRAY2BGR);
    } else {
        output = frame.workingMat.clone();
    }

    int lineCount = 0;
    const double theta = m_thetaDeg * CV_PI / 180.0;
    if (m_mode == "standard") {
        std::vector<cv::Vec2f> lines;
        cv::HoughLines(edges, lines, m_rho, theta, m_threshold);
        lineCount = static_cast<int>(lines.size());
        for (const cv::Vec2f& line : lines) {
            const float rho = line[0];
            const float angle = line[1];
            const double a = std::cos(angle);
            const double b = std::sin(angle);
            const double x0 = a * rho;
            const double y0 = b * rho;
            cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
            cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
            cv::line(output, pt1, pt2, cv::Scalar(0, 255, 0), 2);
        }
    } else {
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(edges, lines, m_rho, theta, m_threshold, m_minLineLength, m_maxLineGap);
        lineCount = static_cast<int>(lines.size());
        for (const cv::Vec4i& line : lines) {
            cv::line(output, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(0, 255, 0), 2);
        }
    }

    frame.metrics.insert("hough_line_count", lineCount);
    frame.workingMat = output;
    return {};
}
