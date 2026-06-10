#include "core/operators/builtin/PerspectiveTransformOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int interpolationFromString(const QString& value)
{
    if (value == "nearest") return cv::INTER_NEAREST;
    if (value == "cubic") return cv::INTER_CUBIC;
    if (value == "area") return cv::INTER_AREA;
    if (value == "lanczos4") return cv::INTER_LANCZOS4;
    return cv::INTER_LINEAR;
}

int borderTypeFromStringPerspective(const QString& value)
{
    if (value == "replicate") return cv::BORDER_REPLICATE;
    if (value == "reflect") return cv::BORDER_REFLECT;
    if (value == "reflect101") return cv::BORDER_REFLECT_101;
    return cv::BORDER_CONSTANT;
}

bool loadPerspectivePointsFromArtifacts(const QVariantMap& artifacts, std::vector<cv::Point2f>* points)
{
    if (!points) {
        return false;
    }

    const QVariantList pointList = artifacts.value("perspective_points").toList();
    if (pointList.size() != 4) {
        return false;
    }

    points->clear();
    for (const QVariant& pointValue : pointList) {
        const QVariantMap map = pointValue.toMap();
        if (!map.contains("x") || !map.contains("y")) {
            return false;
        }
        points->push_back(cv::Point2f(
            static_cast<float>(map.value("x").toDouble()),
            static_cast<float>(map.value("y").toDouble())
        ));
    }
    return points->size() == 4;
}
}

QString PerspectiveTransformOperator::id() const
{
    return "builtin.perspective_transform";
}

QString PerspectiveTransformOperator::displayName() const
{
    return "Perspective Transform";
}

StepSchema PerspectiveTransformOperator::schema() const
{
    return {
        "builtin.perspective_transform",
        "Perspective Transform",
        {
            {"outputWidth", "Output Width", "输出设置", StepParameterType::Integer, 1024, 1, 8192, 1, {}, {}, {}},
            {"outputHeight", "Output Height", "输出设置", StepParameterType::Integer, 768, 1, 8192, 1, {}, {}, {}},
            {"interpolation", "Interpolation", "输出设置", StepParameterType::Choice, "linear", {}, {}, {}, {}, {}, {
                {"nearest", "Nearest"},
                {"linear", "Linear"},
                {"cubic", "Cubic"},
                {"area", "Area"},
                {"lanczos4", "Lanczos4"},
            }},
            {"borderType", "Border Type", "输出设置", StepParameterType::Choice, "constant", {}, {}, {}, {}, {}, {
                {"constant", "Constant"},
                {"replicate", "Replicate"},
                {"reflect", "Reflect"},
                {"reflect101", "Reflect 101"},
            }},
            {"borderValue", "Border Value", "输出设置", StepParameterType::Integer, 0, 0, 255, 1, "borderType=constant", {}, {}},
        }
    };
}

QVariantMap PerspectiveTransformOperator::parameterValues() const
{
    return {
        {"outputWidth", m_outputWidth},
        {"outputHeight", m_outputHeight},
        {"interpolation", m_interpolation},
        {"borderType", m_borderType},
        {"borderValue", m_borderValue},
    };
}

void PerspectiveTransformOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("outputWidth")) m_outputWidth = values.value("outputWidth").toInt();
    if (values.contains("outputHeight")) m_outputHeight = values.value("outputHeight").toInt();
    if (values.contains("interpolation")) m_interpolation = values.value("interpolation").toString();
    if (values.contains("borderType")) m_borderType = values.value("borderType").toString();
    if (values.contains("borderValue")) m_borderValue = values.value("borderValue").toInt();
}

StepResult PerspectiveTransformOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    std::vector<cv::Point2f> srcPoints;
    if (!loadPerspectivePointsFromArtifacts(frame.artifacts, &srcPoints)) {
        return {false, "Perspective transform requires 4 source points in frame.artifacts[perspective_points]"};
    }

    std::vector<cv::Point2f> dstPoints{
        cv::Point2f(0.0f, 0.0f),
        cv::Point2f(static_cast<float>(m_outputWidth - 1), 0.0f),
        cv::Point2f(static_cast<float>(m_outputWidth - 1), static_cast<float>(m_outputHeight - 1)),
        cv::Point2f(0.0f, static_cast<float>(m_outputHeight - 1))
    };

    const cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warped;
    cv::warpPerspective(
        frame.workingMat,
        warped,
        perspectiveMatrix,
        cv::Size(std::max(1, m_outputWidth), std::max(1, m_outputHeight)),
        interpolationFromString(m_interpolation),
        borderTypeFromStringPerspective(m_borderType),
        cv::Scalar(m_borderValue, m_borderValue, m_borderValue)
    );
    frame.workingMat = warped;
    return {};
}
