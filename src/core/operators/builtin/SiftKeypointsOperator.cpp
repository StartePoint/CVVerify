#include "core/operators/builtin/SiftKeypointsOperator.h"

#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

QString SiftKeypointsOperator::id() const
{
    return "builtin.sift_keypoints";
}

QString SiftKeypointsOperator::displayName() const
{
    return "SIFT Keypoints";
}

StepSchema SiftKeypointsOperator::schema() const
{
    return {
        "builtin.sift_keypoints",
        "SIFT Keypoints",
        {
            {"nFeatures", "N Features", "检测设置", StepParameterType::Integer, 0, 0, 100000, 1, {}, "0 means unlimited", {}},
            {"nOctaveLayers", "Octave Layers", "检测设置", StepParameterType::Integer, 3, 1, 8, 1, {}, {}, {}},
            {"contrastThreshold", "Contrast Threshold", "检测设置", StepParameterType::Double, 0.04, 0.001, 1.0, 0.001, {}, {}, {}},
            {"edgeThreshold", "Edge Threshold", "检测设置", StepParameterType::Double, 10.0, 1.0, 100.0, 0.5, {}, {}, {}},
            {"sigma", "Sigma", "检测设置", StepParameterType::Double, 1.6, 0.1, 10.0, 0.1, {}, {}, {}},
            {"drawRichKeypoints", "Draw Rich Keypoints", "绘制设置", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
            {"maxDisplayCount", "Max Display Count", "绘制设置", StepParameterType::Integer, 300, 1, 10000, 1, {}, {}, {}},
        }
    };
}

QVariantMap SiftKeypointsOperator::parameterValues() const
{
    return {
        {"nFeatures", m_nFeatures},
        {"nOctaveLayers", m_nOctaveLayers},
        {"contrastThreshold", m_contrastThreshold},
        {"edgeThreshold", m_edgeThreshold},
        {"sigma", m_sigma},
        {"drawRichKeypoints", m_drawRichKeypoints},
        {"maxDisplayCount", m_maxDisplayCount},
    };
}

void SiftKeypointsOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("nFeatures")) m_nFeatures = values.value("nFeatures").toInt();
    if (values.contains("nOctaveLayers")) m_nOctaveLayers = values.value("nOctaveLayers").toInt();
    if (values.contains("contrastThreshold")) m_contrastThreshold = values.value("contrastThreshold").toDouble();
    if (values.contains("edgeThreshold")) m_edgeThreshold = values.value("edgeThreshold").toDouble();
    if (values.contains("sigma")) m_sigma = values.value("sigma").toDouble();
    if (values.contains("drawRichKeypoints")) m_drawRichKeypoints = values.value("drawRichKeypoints").toBool();
    if (values.contains("maxDisplayCount")) m_maxDisplayCount = values.value("maxDisplayCount").toInt();
}

StepResult SiftKeypointsOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat gray;
    if (frame.workingMat.channels() == 1) {
        gray = frame.workingMat;
    } else {
        cv::cvtColor(frame.workingMat, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Ptr<cv::SIFT> sift = cv::SIFT::create(m_nFeatures, m_nOctaveLayers, m_contrastThreshold, m_edgeThreshold, m_sigma);
    std::vector<cv::KeyPoint> keypoints;
    sift->detect(gray, keypoints);

    if (static_cast<int>(keypoints.size()) > m_maxDisplayCount) {
        keypoints.resize(m_maxDisplayCount);
    }

    cv::Mat output;
    const cv::DrawMatchesFlags drawFlags = m_drawRichKeypoints
        ? cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS
        : cv::DrawMatchesFlags::DEFAULT;
    cv::drawKeypoints(frame.workingMat, keypoints, output, cv::Scalar(0, 255, 0), drawFlags);
    frame.metrics.insert("sift_keypoint_count", static_cast<int>(keypoints.size()));
    frame.workingMat = output;
    return {};
}
