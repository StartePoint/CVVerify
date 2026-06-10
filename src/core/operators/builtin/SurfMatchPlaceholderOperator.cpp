#include "core/operators/builtin/SurfMatchPlaceholderOperator.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace {
cv::Mat ensureGrayForMatching(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

cv::Ptr<cv::Feature2D> createMatchingBackend(const QString& backend)
{
    if (backend == "orb") {
        return cv::ORB::create();
    }
    if (backend == "akaze") {
        return cv::AKAZE::create();
    }
    return cv::SIFT::create();
}

bool isBinaryDescriptorBackend(const QString& backend)
{
    return backend == "orb" || backend == "akaze";
}
}

QString SurfMatchPlaceholderOperator::id() const
{
    return "builtin.surf_match";
}

QString SurfMatchPlaceholderOperator::displayName() const
{
    return "SURF Match";
}

StepSchema SurfMatchPlaceholderOperator::schema() const
{
    return {
        "builtin.surf_match",
        "SURF Match",
        {
            {"secondImageSource", "Second Image Source", "输入设置", StepParameterType::Choice, "external_pick", {}, {}, {}, {}, {}, {
                {"external_pick", "External Pick"},
                {"current_workspace_cache", "Current Workspace Cache"},
            }},
            {"secondaryImagePath", "Secondary Image Path", "输入设置", StepParameterType::String, "", {}, {}, {}, "secondImageSource=external_pick", "Choose the second image used for feature matching", {}},
            {"backend", "Backend", "后端设置", StepParameterType::Choice, "sift", {}, {}, {}, {}, "Current SDK falls back to sift/orb/akaze", {
                {"surf", "SURF"},
                {"sift", "SIFT"},
                {"orb", "ORB"},
                {"akaze", "AKAZE"},
            }},
            {"matcherType", "Matcher Type", "匹配设置", StepParameterType::Choice, "bf", {}, {}, {}, {}, {}, {
                {"bf", "BF"},
                {"flann", "FLANN"},
            }},
            {"ratioTest", "Ratio Test", "匹配设置", StepParameterType::Double, 0.75, 0.1, 1.0, 0.01, {}, {}, {}},
            {"crossCheck", "Cross Check", "匹配设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
            {"maxMatches", "Max Matches", "匹配设置", StepParameterType::Integer, 80, 1, 5000, 1, {}, {}, {}},
            {"useRansac", "Use RANSAC", "几何验证", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
            {"ransacReprojThreshold", "RANSAC Reproj Threshold", "几何验证", StepParameterType::Double, 3.0, 0.1, 50.0, 0.1, "useRansac=true", {}, {}},
            {"showInliersOnly", "Show Inliers Only", "输出设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap SurfMatchPlaceholderOperator::parameterValues() const
{
    return {
        {"secondImageSource", m_secondImageSource},
        {"secondaryImagePath", m_secondaryImagePath},
        {"backend", m_backend},
        {"matcherType", m_matcherType},
        {"ratioTest", m_ratioTest},
        {"crossCheck", m_crossCheck},
        {"maxMatches", m_maxMatches},
        {"useRansac", m_useRansac},
        {"ransacReprojThreshold", m_ransacReprojThreshold},
        {"showInliersOnly", m_showInliersOnly},
    };
}

void SurfMatchPlaceholderOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("secondImageSource")) m_secondImageSource = values.value("secondImageSource").toString();
    if (values.contains("secondaryImagePath")) m_secondaryImagePath = values.value("secondaryImagePath").toString();
    if (values.contains("backend")) m_backend = values.value("backend").toString();
    if (values.contains("matcherType")) m_matcherType = values.value("matcherType").toString();
    if (values.contains("ratioTest")) m_ratioTest = values.value("ratioTest").toDouble();
    if (values.contains("crossCheck")) m_crossCheck = values.value("crossCheck").toBool();
    if (values.contains("maxMatches")) m_maxMatches = values.value("maxMatches").toInt();
    if (values.contains("useRansac")) m_useRansac = values.value("useRansac").toBool();
    if (values.contains("ransacReprojThreshold")) m_ransacReprojThreshold = values.value("ransacReprojThreshold").toDouble();
    if (values.contains("showInliersOnly")) m_showInliersOnly = values.value("showInliersOnly").toBool();
}

StepResult SurfMatchPlaceholderOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    QString secondaryPath;
    if (m_secondImageSource == "external_pick") {
        secondaryPath = m_secondaryImagePath.trimmed();
    } else if (m_secondImageSource == "current_workspace_cache") {
        secondaryPath = frame.artifacts.value("workspace_cached_image_path").toString().trimmed();
    }
    if (secondaryPath.isEmpty()) {
        secondaryPath = frame.artifacts.value("secondary_image_path").toString().trimmed();
    }
    if (secondaryPath.isEmpty()) {
        return {false, "Missing secondary image path for feature matching"};
    }

    const cv::Mat secondaryImage = cv::imread(secondaryPath.toStdString(), cv::IMREAD_COLOR);
    if (secondaryImage.empty()) {
        return {false, "Failed to load secondary image for feature matching"};
    }

    const cv::Ptr<cv::Feature2D> feature = createMatchingBackend(m_backend);
    if (feature.empty()) {
        return {false, "Failed to create matching backend"};
    }

    const cv::Mat firstGray = ensureGrayForMatching(frame.workingMat);
    const cv::Mat secondGray = ensureGrayForMatching(secondaryImage);

    std::vector<cv::KeyPoint> keypoints1;
    std::vector<cv::KeyPoint> keypoints2;
    cv::Mat descriptors1;
    cv::Mat descriptors2;
    feature->detectAndCompute(firstGray, cv::noArray(), keypoints1, descriptors1);
    feature->detectAndCompute(secondGray, cv::noArray(), keypoints2, descriptors2);

    if (descriptors1.empty() || descriptors2.empty()) {
        return {false, "Failed to compute descriptors for feature matching"};
    }

    std::vector<cv::DMatch> goodMatches;
    if (m_matcherType == "flann" && !isBinaryDescriptorBackend(m_backend)) {
        cv::Mat desc1Float;
        cv::Mat desc2Float;
        descriptors1.convertTo(desc1Float, CV_32F);
        descriptors2.convertTo(desc2Float, CV_32F);
        cv::FlannBasedMatcher matcher;
        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher.knnMatch(desc1Float, desc2Float, knnMatches, 2);
        for (const auto& knn : knnMatches) {
            if (knn.size() >= 2 && knn[0].distance < m_ratioTest * knn[1].distance) {
                goodMatches.push_back(knn[0]);
            }
        }
    } else {
        const int normType = isBinaryDescriptorBackend(m_backend) ? cv::NORM_HAMMING : cv::NORM_L2;
        cv::BFMatcher matcher(normType, m_crossCheck);
        if (m_crossCheck) {
            matcher.match(descriptors1, descriptors2, goodMatches);
        } else {
            std::vector<std::vector<cv::DMatch>> knnMatches;
            matcher.knnMatch(descriptors1, descriptors2, knnMatches, 2);
            for (const auto& knn : knnMatches) {
                if (knn.size() >= 2 && knn[0].distance < m_ratioTest * knn[1].distance) {
                    goodMatches.push_back(knn[0]);
                }
            }
        }
    }

    if (static_cast<int>(goodMatches.size()) > m_maxMatches) {
        goodMatches.resize(m_maxMatches);
    }

    std::vector<char> matchMask;
    if (m_useRansac && goodMatches.size() >= 4) {
        std::vector<cv::Point2f> points1;
        std::vector<cv::Point2f> points2;
        for (const cv::DMatch& match : goodMatches) {
            points1.push_back(keypoints1[match.queryIdx].pt);
            points2.push_back(keypoints2[match.trainIdx].pt);
        }
        cv::findHomography(points1, points2, cv::RANSAC, m_ransacReprojThreshold, matchMask);
    }

    std::vector<cv::DMatch> displayMatches;
    if (m_showInliersOnly && !matchMask.empty()) {
        for (std::size_t index = 0; index < goodMatches.size(); ++index) {
            if (index < matchMask.size() && matchMask[index]) {
                displayMatches.push_back(goodMatches[index]);
            }
        }
    } else {
        displayMatches = goodMatches;
    }

    cv::Mat output;
    cv::drawMatches(
        frame.workingMat,
        keypoints1,
        secondaryImage,
        keypoints2,
        displayMatches,
        output,
        cv::Scalar::all(-1),
        cv::Scalar::all(-1),
        matchMask.empty() ? std::vector<char>() : matchMask,
        cv::DrawMatchesFlags::DEFAULT
    );

    frame.metrics.insert("surf_placeholder_match_count", static_cast<int>(displayMatches.size()));
    frame.annotations.insert("feature_backend", m_backend);
    frame.annotations.insert("secondary_image_path", secondaryPath);
    frame.workingMat = output;
    return {};
}
