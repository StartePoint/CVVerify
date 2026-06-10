#include "core/operators/builtin/SurfKeypointsPlaceholderOperator.h"

#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForFeatureDetection(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

cv::Ptr<cv::Feature2D> createFeatureBackend(
    const QString& backend,
    double hessianThreshold,
    int nOctaves,
    int nOctaveLayers,
    bool extended,
    bool upright)
{
    Q_UNUSED(hessianThreshold);
    Q_UNUSED(nOctaves);
    Q_UNUSED(extended);
    Q_UNUSED(upright);

    if (backend == "orb") {
        return cv::ORB::create();
    }
    if (backend == "akaze") {
        return cv::AKAZE::create();
    }
    return cv::SIFT::create(0, std::max(1, nOctaveLayers));
}
}

QString SurfKeypointsPlaceholderOperator::id() const
{
    return "builtin.surf_keypoints";
}

QString SurfKeypointsPlaceholderOperator::displayName() const
{
    return "SURF Keypoints";
}

StepSchema SurfKeypointsPlaceholderOperator::schema() const
{
    return {
        "builtin.surf_keypoints",
        "SURF Keypoints",
        {
            {"backend", "Backend", "后端设置", StepParameterType::Choice, "sift", {}, {}, {}, {}, "Current SDK falls back to sift/orb/akaze", {
                {"surf", "SURF"},
                {"sift", "SIFT"},
                {"orb", "ORB"},
                {"akaze", "AKAZE"},
            }},
            {"hessianThreshold", "Hessian Threshold", "SURF 规范参数", StepParameterType::Double, 400.0, 1.0, 5000.0, 1.0, {}, {}, {}},
            {"nOctaves", "Octaves", "SURF 规范参数", StepParameterType::Integer, 4, 1, 8, 1, {}, {}, {}},
            {"nOctaveLayers", "Octave Layers", "SURF 规范参数", StepParameterType::Integer, 3, 1, 8, 1, {}, {}, {}},
            {"extended", "Extended Descriptor", "SURF 规范参数", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
            {"upright", "Upright", "SURF 规范参数", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
            {"maxDisplayCount", "Max Display Count", "绘制设置", StepParameterType::Integer, 300, 1, 10000, 1, {}, {}, {}},
        }
    };
}

QVariantMap SurfKeypointsPlaceholderOperator::parameterValues() const
{
    return {
        {"backend", m_backend},
        {"hessianThreshold", m_hessianThreshold},
        {"nOctaves", m_nOctaves},
        {"nOctaveLayers", m_nOctaveLayers},
        {"extended", m_extended},
        {"upright", m_upright},
        {"maxDisplayCount", m_maxDisplayCount},
    };
}

void SurfKeypointsPlaceholderOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("backend")) m_backend = values.value("backend").toString();
    if (values.contains("hessianThreshold")) m_hessianThreshold = values.value("hessianThreshold").toDouble();
    if (values.contains("nOctaves")) m_nOctaves = values.value("nOctaves").toInt();
    if (values.contains("nOctaveLayers")) m_nOctaveLayers = values.value("nOctaveLayers").toInt();
    if (values.contains("extended")) m_extended = values.value("extended").toBool();
    if (values.contains("upright")) m_upright = values.value("upright").toBool();
    if (values.contains("maxDisplayCount")) m_maxDisplayCount = values.value("maxDisplayCount").toInt();
}

StepResult SurfKeypointsPlaceholderOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    const cv::Mat gray = ensureGrayForFeatureDetection(frame.workingMat);
    const cv::Ptr<cv::Feature2D> feature = createFeatureBackend(
        m_backend,
        m_hessianThreshold,
        m_nOctaves,
        m_nOctaveLayers,
        m_extended,
        m_upright
    );
    if (feature.empty()) {
        return {false, "Failed to create feature backend"};
    }

    std::vector<cv::KeyPoint> keypoints;
    feature->detect(gray, keypoints);
    if (static_cast<int>(keypoints.size()) > m_maxDisplayCount) {
        keypoints.resize(m_maxDisplayCount);
    }

    cv::Mat output;
    cv::drawKeypoints(frame.workingMat, keypoints, output, cv::Scalar(0, 255, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    frame.metrics.insert("surf_placeholder_keypoint_count", static_cast<int>(keypoints.size()));
    frame.annotations.insert("feature_backend", m_backend);
    frame.workingMat = output;
    return {};
}
