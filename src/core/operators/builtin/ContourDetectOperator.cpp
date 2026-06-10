#include "core/operators/builtin/ContourDetectOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureContourInput(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

int retrievalModeFromString(const QString& value)
{
    if (value == "list") return cv::RETR_LIST;
    if (value == "ccomp") return cv::RETR_CCOMP;
    if (value == "tree") return cv::RETR_TREE;
    return cv::RETR_EXTERNAL;
}

int approxModeFromString(const QString& value)
{
    if (value == "none") return cv::CHAIN_APPROX_NONE;
    if (value == "tc89_l1") return cv::CHAIN_APPROX_TC89_L1;
    if (value == "tc89_kcos") return cv::CHAIN_APPROX_TC89_KCOS;
    return cv::CHAIN_APPROX_SIMPLE;
}
}

QString ContourDetectOperator::id() const
{
    return "builtin.contour_detect";
}

QString ContourDetectOperator::displayName() const
{
    return "Contour Detect";
}

StepSchema ContourDetectOperator::schema() const
{
    return {
        "builtin.contour_detect",
        "Contour Detect",
        {
            {"retrievalMode", "Retrieval Mode", "轮廓设置", StepParameterType::Choice, "external", {}, {}, {}, {}, {}, {
                {"external", "External"},
                {"list", "List"},
                {"ccomp", "CCOMP"},
                {"tree", "Tree"},
            }},
            {"approxMode", "Approx Mode", "轮廓设置", StepParameterType::Choice, "simple", {}, {}, {}, {}, {}, {
                {"none", "None"},
                {"simple", "Simple"},
                {"tc89_l1", "TC89 L1"},
                {"tc89_kcos", "TC89 KCOS"},
            }},
            {"minArea", "Min Area", "轮廓设置", StepParameterType::Double, 10.0, 0.0, 100000000.0, 1.0, {}, {}, {}},
            {"maxArea", "Max Area", "轮廓设置", StepParameterType::Double, 0.0, 0.0, 100000000.0, 1.0, {}, "0 means unlimited", {}},
            {"drawMode", "Draw Mode", "绘制设置", StepParameterType::Choice, "outline", {}, {}, {}, {}, {}, {
                {"outline", "Outline"},
                {"filled", "Filled"},
                {"bbox", "Bounding Box"},
            }},
            {"thickness", "Thickness", "绘制设置", StepParameterType::Integer, 2, 1, 20, 1, "drawMode=outline", {}, {}},
        }
    };
}

QVariantMap ContourDetectOperator::parameterValues() const
{
    return {
        {"retrievalMode", m_retrievalMode},
        {"approxMode", m_approxMode},
        {"minArea", m_minArea},
        {"maxArea", m_maxArea},
        {"drawMode", m_drawMode},
        {"thickness", m_thickness},
    };
}

void ContourDetectOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("retrievalMode")) {
        m_retrievalMode = values.value("retrievalMode").toString();
    }
    if (values.contains("approxMode")) {
        m_approxMode = values.value("approxMode").toString();
    }
    if (values.contains("minArea")) {
        m_minArea = values.value("minArea").toDouble();
    }
    if (values.contains("maxArea")) {
        m_maxArea = values.value("maxArea").toDouble();
    }
    if (values.contains("drawMode")) {
        m_drawMode = values.value("drawMode").toString();
    }
    if (values.contains("thickness")) {
        m_thickness = values.value("thickness").toInt();
    }
}

StepResult ContourDetectOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray = ensureContourInput(frame.workingMat);
    cv::Mat binary;
    if (gray.type() != CV_8UC1) {
        gray.convertTo(gray, CV_8UC1);
    }
    cv::threshold(gray, binary, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary, contours, hierarchy, retrievalModeFromString(m_retrievalMode), approxModeFromString(m_approxMode));

    cv::Mat output;
    if (frame.workingMat.channels() == 1) {
        cv::cvtColor(frame.workingMat, output, cv::COLOR_GRAY2BGR);
    } else {
        output = frame.workingMat.clone();
    }

    int keptCount = 0;
    for (const std::vector<cv::Point>& contour : contours) {
        const double area = cv::contourArea(contour);
        if (area < m_minArea) {
            continue;
        }
        if (m_maxArea > 0.0 && area > m_maxArea) {
            continue;
        }

        ++keptCount;
        if (m_drawMode == "filled") {
            std::vector<std::vector<cv::Point>> singleContour{contour};
            cv::drawContours(output, singleContour, -1, cv::Scalar(0, 255, 0), cv::FILLED);
        } else if (m_drawMode == "bbox") {
            cv::rectangle(output, cv::boundingRect(contour), cv::Scalar(0, 255, 0), 2);
        } else {
            std::vector<std::vector<cv::Point>> singleContour{contour};
            cv::drawContours(output, singleContour, -1, cv::Scalar(0, 255, 0), std::max(1, m_thickness));
        }
    }

    frame.metrics.insert("contour_count", keptCount);
    frame.workingMat = output;
    return {};
}
