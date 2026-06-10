#include "core/operators/builtin/KMeansSegmentationOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Mat convertColorSpaceForKMeans(const cv::Mat& source, const QString& colorSpace)
{
    if (colorSpace == "hsv") {
        cv::Mat converted;
        cv::cvtColor(source, converted, cv::COLOR_BGR2HSV);
        return converted;
    }
    if (colorSpace == "lab") {
        cv::Mat converted;
        cv::cvtColor(source, converted, cv::COLOR_BGR2Lab);
        return converted;
    }
    return source;
}
}

QString KMeansSegmentationOperator::id() const
{
    return "builtin.segment_kmeans";
}

QString KMeansSegmentationOperator::displayName() const
{
    return "K-Means Segmentation";
}

StepSchema KMeansSegmentationOperator::schema() const
{
    return {
        "builtin.segment_kmeans",
        "K-Means Segmentation",
        {
            {"clusterCount", "Cluster Count", "聚类设置", StepParameterType::Integer, 4, 2, 32, 1, {}, {}, {}},
            {"colorSpace", "Color Space", "聚类设置", StepParameterType::Choice, "lab", {}, {}, {}, {}, {}, {
                {"bgr", "BGR"},
                {"hsv", "HSV"},
                {"lab", "Lab"},
            }},
            {"includeSpatialXY", "Include Spatial XY", "聚类设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
            {"spatialWeight", "Spatial Weight", "聚类设置", StepParameterType::Double, 1.0, 0.0, 1000.0, 1.0, "includeSpatialXY=true", {}, {}},
            {"maxIterations", "Max Iterations", "聚类设置", StepParameterType::Integer, 20, 1, 1000, 1, {}, {}, {}},
            {"epsilon", "Epsilon", "聚类设置", StepParameterType::Double, 1.0, 0.0001, 100.0, 0.1, {}, {}, {}},
            {"attempts", "Attempts", "聚类设置", StepParameterType::Integer, 3, 1, 100, 1, {}, {}, {}},
        }
    };
}

QVariantMap KMeansSegmentationOperator::parameterValues() const
{
    return {
        {"clusterCount", m_clusterCount},
        {"colorSpace", m_colorSpace},
        {"includeSpatialXY", m_includeSpatialXY},
        {"spatialWeight", m_spatialWeight},
        {"maxIterations", m_maxIterations},
        {"epsilon", m_epsilon},
        {"attempts", m_attempts},
    };
}

void KMeansSegmentationOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("clusterCount")) m_clusterCount = values.value("clusterCount").toInt();
    if (values.contains("colorSpace")) m_colorSpace = values.value("colorSpace").toString();
    if (values.contains("includeSpatialXY")) m_includeSpatialXY = values.value("includeSpatialXY").toBool();
    if (values.contains("spatialWeight")) m_spatialWeight = values.value("spatialWeight").toDouble();
    if (values.contains("maxIterations")) m_maxIterations = values.value("maxIterations").toInt();
    if (values.contains("epsilon")) m_epsilon = values.value("epsilon").toDouble();
    if (values.contains("attempts")) m_attempts = values.value("attempts").toInt();
}

StepResult KMeansSegmentationOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (frame.workingMat.channels() != 3) {
        return {false, "K-Means segmentation requires a 3-channel color image"};
    }

    const cv::Mat converted = convertColorSpaceForKMeans(frame.workingMat, m_colorSpace);
    cv::Mat samples(converted.rows * converted.cols, m_includeSpatialXY ? 5 : 3, CV_32F);

    for (int row = 0; row < converted.rows; ++row) {
        for (int col = 0; col < converted.cols; ++col) {
            const cv::Vec3b pixel = converted.at<cv::Vec3b>(row, col);
            const int index = row * converted.cols + col;
            samples.at<float>(index, 0) = pixel[0];
            samples.at<float>(index, 1) = pixel[1];
            samples.at<float>(index, 2) = pixel[2];
            if (m_includeSpatialXY) {
                samples.at<float>(index, 3) = static_cast<float>(col * m_spatialWeight);
                samples.at<float>(index, 4) = static_cast<float>(row * m_spatialWeight);
            }
        }
    }

    cv::Mat labels;
    cv::Mat centers;
    cv::kmeans(
        samples,
        std::max(2, m_clusterCount),
        labels,
        cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, std::max(1, m_maxIterations), std::max(0.0001, m_epsilon)),
        std::max(1, m_attempts),
        cv::KMEANS_PP_CENTERS,
        centers
    );

    cv::Mat result(converted.size(), converted.type());
    for (int row = 0; row < converted.rows; ++row) {
        for (int col = 0; col < converted.cols; ++col) {
            const int index = row * converted.cols + col;
            const int clusterIndex = labels.at<int>(index, 0);
            result.at<cv::Vec3b>(row, col) = cv::Vec3b(
                static_cast<uchar>(centers.at<float>(clusterIndex, 0)),
                static_cast<uchar>(centers.at<float>(clusterIndex, 1)),
                static_cast<uchar>(centers.at<float>(clusterIndex, 2))
            );
        }
    }

    if (m_colorSpace == "hsv") {
        cv::cvtColor(result, frame.workingMat, cv::COLOR_HSV2BGR);
    } else if (m_colorSpace == "lab") {
        cv::cvtColor(result, frame.workingMat, cv::COLOR_Lab2BGR);
    } else {
        frame.workingMat = result;
    }
    frame.metrics.insert("kmeans_cluster_count", std::max(2, m_clusterCount));
    return {};
}
