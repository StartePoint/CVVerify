#include "core/operators/builtin/MorphologyOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int morphologyOperationFromString(const QString& value)
{
    if (value == "erode") {
        return -1;
    }
    if (value == "dilate") {
        return -2;
    }
    if (value == "open") {
        return cv::MORPH_OPEN;
    }
    if (value == "close") {
        return cv::MORPH_CLOSE;
    }
    if (value == "gradient") {
        return cv::MORPH_GRADIENT;
    }
    if (value == "tophat") {
        return cv::MORPH_TOPHAT;
    }
    if (value == "blackhat") {
        return cv::MORPH_BLACKHAT;
    }
    return -2;
}

int morphologyShapeFromString(const QString& value)
{
    if (value == "ellipse") {
        return cv::MORPH_ELLIPSE;
    }
    if (value == "cross") {
        return cv::MORPH_CROSS;
    }
    return cv::MORPH_RECT;
}

int borderTypeFromString(const QString& value)
{
    if (value == "constant") {
        return cv::BORDER_CONSTANT;
    }
    if (value == "replicate") {
        return cv::BORDER_REPLICATE;
    }
    if (value == "reflect") {
        return cv::BORDER_REFLECT;
    }
    return cv::BORDER_REFLECT_101;
}
}

QString MorphologyOperator::id() const
{
    return "builtin.morphology";
}

QString MorphologyOperator::displayName() const
{
    return "Morphology";
}

StepSchema MorphologyOperator::schema() const
{
    return {
        "builtin.morphology",
        "Morphology",
        {
            {"operation", "Operation", "操作设置", StepParameterType::Choice, "dilate", {}, {}, {}, {}, {}, {
                {"erode", "Erode"},
                {"dilate", "Dilate"},
                {"open", "Open"},
                {"close", "Close"},
                {"gradient", "Gradient"},
                {"tophat", "Top Hat"},
                {"blackhat", "Black Hat"},
            }},
            {"kernelShape", "Kernel Shape", "核设置", StepParameterType::Choice, "rect", {}, {}, {}, {}, {}, {
                {"rect", "Rectangle"},
                {"ellipse", "Ellipse"},
                {"cross", "Cross"},
            }},
            {"kernelWidth", "Kernel Width", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
            {"kernelHeight", "Kernel Height", "核设置", StepParameterType::Integer, 3, 1, 99, 1, {}, {}, {}},
            {"iterations", "Iterations", "操作设置", StepParameterType::Integer, 1, 1, 20, 1, {}, {}, {}},
            {"borderType", "Border Type", "边界设置", StepParameterType::Choice, "reflect101", {}, {}, {}, {}, {}, {
                {"constant", "Constant"},
                {"replicate", "Replicate"},
                {"reflect", "Reflect"},
                {"reflect101", "Reflect 101"},
            }},
            {"borderValue", "Border Value", "边界设置", StepParameterType::Integer, 0, 0, 255, 1, "borderType=constant", {}, {}},
        }
    };
}

QVariantMap MorphologyOperator::parameterValues() const
{
    return {
        {"operation", m_operation},
        {"kernelShape", m_kernelShape},
        {"kernelWidth", m_kernelWidth},
        {"kernelHeight", m_kernelHeight},
        {"iterations", m_iterations},
        {"borderType", m_borderType},
        {"borderValue", m_borderValue},
    };
}

void MorphologyOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("operation")) {
        m_operation = values.value("operation").toString();
    }
    if (values.contains("kernelShape")) {
        m_kernelShape = values.value("kernelShape").toString();
    }
    if (values.contains("kernelWidth")) {
        m_kernelWidth = values.value("kernelWidth").toInt();
    }
    if (values.contains("kernelHeight")) {
        m_kernelHeight = values.value("kernelHeight").toInt();
    }
    if (values.contains("iterations")) {
        m_iterations = values.value("iterations").toInt();
    }
    if (values.contains("borderType")) {
        m_borderType = values.value("borderType").toString();
    }
    if (values.contains("borderValue")) {
        m_borderValue = values.value("borderValue").toInt();
    }
}

StepResult MorphologyOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat kernel = cv::getStructuringElement(
        morphologyShapeFromString(m_kernelShape),
        cv::Size(std::max(1, m_kernelWidth), std::max(1, m_kernelHeight))
    );
    cv::Mat result;
    const int borderType = borderTypeFromString(m_borderType);
    const cv::Scalar borderValue(m_borderValue, m_borderValue, m_borderValue);

    const int operation = morphologyOperationFromString(m_operation);
    if (operation == -1) {
        cv::erode(frame.workingMat, result, kernel, cv::Point(-1, -1), std::max(1, m_iterations), borderType, borderValue);
    } else if (operation == -2) {
        cv::dilate(frame.workingMat, result, kernel, cv::Point(-1, -1), std::max(1, m_iterations), borderType, borderValue);
    } else {
        cv::morphologyEx(
            frame.workingMat,
            result,
            operation,
            kernel,
            cv::Point(-1, -1),
            std::max(1, m_iterations),
            borderType,
            borderValue
        );
    }

    frame.workingMat = result;
    return {};
}
