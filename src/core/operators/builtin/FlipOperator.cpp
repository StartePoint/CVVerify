#include "core/operators/builtin/FlipOperator.h"

#include <opencv2/imgproc.hpp>

namespace {
int flipCodeFromString(const QString& value)
{
    if (value == "vertical") return 0;
    if (value == "both") return -1;
    return 1;
}
}

QString FlipOperator::id() const
{
    return "builtin.flip";
}

QString FlipOperator::displayName() const
{
    return "Flip";
}

StepSchema FlipOperator::schema() const
{
    return {
        "builtin.flip",
        "Flip",
        {
            {"flipCode", "Flip Code", "翻转设置", StepParameterType::Choice, "horizontal", {}, {}, {}, {}, {}, {
                {"horizontal", "Horizontal"},
                {"vertical", "Vertical"},
                {"both", "Both"},
            }},
        }
    };
}

QVariantMap FlipOperator::parameterValues() const
{
    return {
        {"flipCode", m_flipCode},
    };
}

void FlipOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("flipCode")) {
        m_flipCode = values.value("flipCode").toString();
    }
}

StepResult FlipOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat flipped;
    cv::flip(frame.workingMat, flipped, flipCodeFromString(m_flipCode));
    frame.workingMat = flipped;
    return {};
}
