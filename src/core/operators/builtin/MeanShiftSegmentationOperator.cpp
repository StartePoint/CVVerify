#include "core/operators/builtin/MeanShiftSegmentationOperator.h"

#include <opencv2/imgproc.hpp>

QString MeanShiftSegmentationOperator::id() const
{
    return "builtin.segment_meanshift";
}

QString MeanShiftSegmentationOperator::displayName() const
{
    return "MeanShift Segmentation";
}

StepSchema MeanShiftSegmentationOperator::schema() const
{
    return {
        "builtin.segment_meanshift",
        "MeanShift Segmentation",
        {
            {"spatialRadius", "Spatial Radius", "分割设置", StepParameterType::Double, 10.0, 1.0, 100.0, 1.0, {}, {}, {}},
            {"colorRadius", "Color Radius", "分割设置", StepParameterType::Double, 20.0, 1.0, 255.0, 1.0, {}, {}, {}},
            {"maxLevel", "Max Level", "分割设置", StepParameterType::Integer, 1, 0, 4, 1, {}, {}, {}},
            {"termMaxCount", "Term Max Count", "终止条件", StepParameterType::Integer, 5, 1, 100, 1, {}, {}, {}},
            {"termEpsilon", "Term Epsilon", "终止条件", StepParameterType::Double, 1.0, 0.001, 20.0, 0.1, {}, {}, {}},
        }
    };
}

QVariantMap MeanShiftSegmentationOperator::parameterValues() const
{
    return {
        {"spatialRadius", m_spatialRadius},
        {"colorRadius", m_colorRadius},
        {"maxLevel", m_maxLevel},
        {"termMaxCount", m_termMaxCount},
        {"termEpsilon", m_termEpsilon},
    };
}

void MeanShiftSegmentationOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("spatialRadius")) m_spatialRadius = values.value("spatialRadius").toDouble();
    if (values.contains("colorRadius")) m_colorRadius = values.value("colorRadius").toDouble();
    if (values.contains("maxLevel")) m_maxLevel = values.value("maxLevel").toInt();
    if (values.contains("termMaxCount")) m_termMaxCount = values.value("termMaxCount").toInt();
    if (values.contains("termEpsilon")) m_termEpsilon = values.value("termEpsilon").toDouble();
}

StepResult MeanShiftSegmentationOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }
    if (frame.workingMat.channels() != 3) {
        return {false, "MeanShift segmentation requires a 3-channel color image"};
    }

    cv::Mat result;
    cv::pyrMeanShiftFiltering(
        frame.workingMat,
        result,
        m_spatialRadius,
        m_colorRadius,
        m_maxLevel,
        cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, std::max(1, m_termMaxCount), std::max(0.001, m_termEpsilon))
    );
    frame.workingMat = result;
    return {};
}
