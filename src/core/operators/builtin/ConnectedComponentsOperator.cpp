#include "core/operators/builtin/ConnectedComponentsOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureBinaryComponentInput(const cv::Mat& source)
{
    cv::Mat gray;
    if (source.channels() == 1) {
        gray = source;
    } else {
        cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat binary;
    cv::threshold(gray, binary, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return binary;
}

cv::Vec3b colorForLabel(int label)
{
    const uchar b = static_cast<uchar>((label * 53) % 255);
    const uchar g = static_cast<uchar>((label * 97) % 255);
    const uchar r = static_cast<uchar>((label * 193) % 255);
    return cv::Vec3b(b, g, r);
}
}

QString ConnectedComponentsOperator::id() const
{
    return "builtin.connected_components";
}

QString ConnectedComponentsOperator::displayName() const
{
    return "Connected Components";
}

StepSchema ConnectedComponentsOperator::schema() const
{
    return {
        "builtin.connected_components",
        "Connected Components",
        {
            {"connectivity", "Connectivity", "检测设置", StepParameterType::Choice, "8", {}, {}, {}, {}, {}, {
                {"4", "4"},
                {"8", "8"},
            }},
            {"minArea", "Min Area", "筛选设置", StepParameterType::Integer, 1, 1, 100000000, 1, {}, {}, {}},
            {"maxArea", "Max Area", "筛选设置", StepParameterType::Integer, 0, 0, 100000000, 1, {}, "0 means unlimited", {}},
            {"colorize", "Colorize Labels", "输出设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
            {"drawBBox", "Draw Bounding Box", "输出设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap ConnectedComponentsOperator::parameterValues() const
{
    return {
        {"connectivity", m_connectivity},
        {"minArea", m_minArea},
        {"maxArea", m_maxArea},
        {"colorize", m_colorize},
        {"drawBBox", m_drawBBox},
    };
}

void ConnectedComponentsOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("connectivity")) {
        m_connectivity = values.value("connectivity").toInt();
    }
    if (values.contains("minArea")) {
        m_minArea = values.value("minArea").toInt();
    }
    if (values.contains("maxArea")) {
        m_maxArea = values.value("maxArea").toInt();
    }
    if (values.contains("colorize")) {
        m_colorize = values.value("colorize").toBool();
    }
    if (values.contains("drawBBox")) {
        m_drawBBox = values.value("drawBBox").toBool();
    }
}

StepResult ConnectedComponentsOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat binary = ensureBinaryComponentInput(frame.workingMat);
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int componentCount = cv::connectedComponentsWithStats(binary, labels, stats, centroids, m_connectivity, CV_32S);

    cv::Mat output(binary.rows, binary.cols, CV_8UC3, cv::Scalar(0, 0, 0));
    int keptCount = 0;

    for (int label = 1; label < componentCount; ++label) {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area < m_minArea) {
            continue;
        }
        if (m_maxArea > 0 && area > m_maxArea) {
            continue;
        }

        ++keptCount;
        if (m_colorize) {
            const cv::Vec3b color = colorForLabel(label);
            for (int row = 0; row < labels.rows; ++row) {
                int* labelPtr = labels.ptr<int>(row);
                cv::Vec3b* outputPtr = output.ptr<cv::Vec3b>(row);
                for (int col = 0; col < labels.cols; ++col) {
                    if (labelPtr[col] == label) {
                        outputPtr[col] = color;
                    }
                }
            }
        }

        if (m_drawBBox) {
            const cv::Rect box(
                stats.at<int>(label, cv::CC_STAT_LEFT),
                stats.at<int>(label, cv::CC_STAT_TOP),
                stats.at<int>(label, cv::CC_STAT_WIDTH),
                stats.at<int>(label, cv::CC_STAT_HEIGHT)
            );
            cv::rectangle(output, box, cv::Scalar(255, 255, 255), 1);
        }
    }

    frame.metrics.insert("connected_component_count", keptCount);
    frame.workingMat = output;
    return {};
}
