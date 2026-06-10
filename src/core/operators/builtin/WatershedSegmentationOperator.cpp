#include "core/operators/builtin/WatershedSegmentationOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
cv::Scalar scalarFromHexColor(const QString& hex)
{
    QString value = hex.trimmed();
    if (value.startsWith('#')) {
        value.remove(0, 1);
    }
    if (value.size() != 6) {
        return cv::Scalar(85, 45, 255);
    }

    const bool ok = true;
    const int r = value.mid(0, 2).toInt(nullptr, 16);
    const int g = value.mid(2, 2).toInt(nullptr, 16);
    const int b = value.mid(4, 2).toInt(nullptr, 16);
    Q_UNUSED(ok);
    return cv::Scalar(b, g, r);
}
}

QString WatershedSegmentationOperator::id() const
{
    return "builtin.segment_watershed";
}

QString WatershedSegmentationOperator::displayName() const
{
    return "Watershed Segmentation";
}

StepSchema WatershedSegmentationOperator::schema() const
{
    return {
        "builtin.segment_watershed",
        "Watershed Segmentation",
        {
            {"markerMode", "Marker Mode", "模式设置", StepParameterType::Choice, "distance_transform_auto", {}, {}, {}, {}, {}, {
                {"distance_transform_auto", "Distance Transform Auto"},
                {"contour_auto", "Contour Auto"},
            }},
            {"openingKernel", "Opening Kernel", "预处理设置", StepParameterType::Integer, 3, 1, 31, 1, {}, {}, {}},
            {"dilateIterations", "Dilate Iterations", "预处理设置", StepParameterType::Integer, 3, 1, 20, 1, {}, {}, {}},
            {"distanceThresholdRatio", "Distance Threshold Ratio", "Marker 设置", StepParameterType::Double, 0.35, 0.01, 1.0, 0.01, "markerMode=distance_transform_auto", {}, {}},
            {"minMarkerArea", "Min Marker Area", "Marker 设置", StepParameterType::Integer, 32, 1, 1000000, 1, {}, {}, {}},
            {"boundaryColor", "Boundary Color", "输出设置", StepParameterType::String, "#ff2d55", {}, {}, {}, {}, {}, {}},
        }
    };
}

QVariantMap WatershedSegmentationOperator::parameterValues() const
{
    return {
        {"markerMode", m_markerMode},
        {"openingKernel", m_openingKernel},
        {"dilateIterations", m_dilateIterations},
        {"distanceThresholdRatio", m_distanceThresholdRatio},
        {"minMarkerArea", m_minMarkerArea},
        {"boundaryColor", m_boundaryColor},
    };
}

void WatershedSegmentationOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("markerMode")) m_markerMode = values.value("markerMode").toString();
    if (values.contains("openingKernel")) m_openingKernel = values.value("openingKernel").toInt();
    if (values.contains("dilateIterations")) m_dilateIterations = values.value("dilateIterations").toInt();
    if (values.contains("distanceThresholdRatio")) m_distanceThresholdRatio = values.value("distanceThresholdRatio").toDouble();
    if (values.contains("minMarkerArea")) m_minMarkerArea = values.value("minMarkerArea").toInt();
    if (values.contains("boundaryColor")) m_boundaryColor = values.value("boundaryColor").toString();
}

StepResult WatershedSegmentationOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (frame.workingMat.channels() != 3) {
        return {false, "Watershed segmentation requires a 3-channel color image"};
    }

    cv::Mat gray;
    cv::cvtColor(frame.workingMat, gray, cv::COLOR_BGR2GRAY);

    cv::Mat binary;
    cv::threshold(gray, binary, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(std::max(1, m_openingKernel), std::max(1, m_openingKernel)));
    cv::Mat opened;
    cv::morphologyEx(binary, opened, cv::MORPH_OPEN, kernel);

    cv::Mat sureBg;
    cv::dilate(opened, sureBg, kernel, cv::Point(-1, -1), std::max(1, m_dilateIterations));

    cv::Mat markers;
    if (m_markerMode == "contour_auto") {
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(opened, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        markers = cv::Mat::zeros(opened.size(), CV_32S);
        int label = 1;
        for (const std::vector<cv::Point>& contour : contours) {
            if (cv::contourArea(contour) < m_minMarkerArea) {
                continue;
            }
            std::vector<std::vector<cv::Point>> singleContour{contour};
            cv::drawContours(markers, singleContour, -1, cv::Scalar(label), cv::FILLED);
            ++label;
        }
    } else {
        cv::Mat dist;
        cv::distanceTransform(opened, dist, cv::DIST_L2, 5);
        cv::Mat sureFg;
        cv::threshold(dist, sureFg, m_distanceThresholdRatio * cv::norm(dist, cv::NORM_INF), 255, cv::THRESH_BINARY);
        sureFg.convertTo(sureFg, CV_8U);

        int componentCount = cv::connectedComponents(sureFg, markers);
        Q_UNUSED(componentCount);
        markers += 1;
        for (int row = 0; row < markers.rows; ++row) {
            for (int col = 0; col < markers.cols; ++col) {
                if (sureBg.at<uchar>(row, col) == 0) {
                    markers.at<int>(row, col) = 0;
                }
            }
        }
    }

    cv::watershed(frame.workingMat, markers);

    cv::Mat output = frame.workingMat.clone();
    const cv::Scalar boundaryColor = scalarFromHexColor(m_boundaryColor);
    int regionCount = 0;
    for (int row = 0; row < markers.rows; ++row) {
        for (int col = 0; col < markers.cols; ++col) {
            const int label = markers.at<int>(row, col);
            if (label == -1) {
                output.at<cv::Vec3b>(row, col) = cv::Vec3b(
                    static_cast<uchar>(boundaryColor[0]),
                    static_cast<uchar>(boundaryColor[1]),
                    static_cast<uchar>(boundaryColor[2])
                );
            } else if (label > regionCount) {
                regionCount = label;
            }
        }
    }

    frame.metrics.insert("watershed_region_count", regionCount);
    frame.workingMat = output;
    return {};
}
