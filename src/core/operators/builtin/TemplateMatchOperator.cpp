#include "core/operators/builtin/TemplateMatchOperator.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "infra/opencv/OpenCvImageIO.h"

namespace {
int matchMethodFromString(const QString& value)
{
    if (value == "sqdiff") return cv::TM_SQDIFF;
    if (value == "sqdiff_normed") return cv::TM_SQDIFF_NORMED;
    if (value == "ccorr") return cv::TM_CCORR;
    if (value == "ccorr_normed") return cv::TM_CCORR_NORMED;
    if (value == "ccoeff") return cv::TM_CCOEFF;
    return cv::TM_CCOEFF_NORMED;
}

bool isLowerBetterMatchMethod(int method)
{
    return method == cv::TM_SQDIFF || method == cv::TM_SQDIFF_NORMED;
}

cv::Mat ensureMatchingInput(const cv::Mat& source)
{
    if (source.channels() == 1) {
        return source;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
    return gray;
}
}

QString TemplateMatchOperator::id() const
{
    return "builtin.template_match";
}

QString TemplateMatchOperator::displayName() const
{
    return "Template Match";
}

StepSchema TemplateMatchOperator::schema() const
{
    return {
        "builtin.template_match",
        "Template Match",
        {
            {"templateImagePath", "Template Image Path", "输入设置", StepParameterType::String, "", {}, {}, {}, {}, "Choose the template image used for matching", {}},
            {"matchMethod", "Match Method", "匹配设置", StepParameterType::Choice, "ccoeff_normed", {}, {}, {}, {}, {}, {
                {"sqdiff", "SQDIFF"},
                {"sqdiff_normed", "SQDIFF Normed"},
                {"ccorr", "CCORR"},
                {"ccorr_normed", "CCORR Normed"},
                {"ccoeff", "CCOEFF"},
                {"ccoeff_normed", "CCOEFF Normed"},
            }},
            {"threshold", "Threshold", "匹配设置", StepParameterType::Double, 0.8, 0.0, 1.0, 0.01, {}, "For SQDIFF methods lower is better; for others higher is better", {}},
        }
    };
}

QVariantMap TemplateMatchOperator::parameterValues() const
{
    return {
        {"templateImagePath", m_templateImagePath},
        {"matchMethod", m_matchMethod},
        {"threshold", m_threshold},
    };
}

void TemplateMatchOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("templateImagePath")) {
        m_templateImagePath = values.value("templateImagePath").toString();
    }
    if (values.contains("matchMethod")) {
        m_matchMethod = values.value("matchMethod").toString();
    }
    if (values.contains("threshold")) {
        m_threshold = values.value("threshold").toDouble();
    }
}

StepResult TemplateMatchOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    const QString templatePath = m_templateImagePath.trimmed();
    if (templatePath.isEmpty()) {
        return {false, "Missing template image path"};
    }

    const cv::Mat templateImage = cv::imread(OpenCvImageIO::toOpenCvFilePath(templatePath), cv::IMREAD_UNCHANGED);
    if (templateImage.empty()) {
        return {false, "Failed to load template image"};
    }

    const cv::Mat sourceGray = ensureMatchingInput(frame.workingMat);
    const cv::Mat templateGray = ensureMatchingInput(templateImage);
    if (templateGray.cols > sourceGray.cols || templateGray.rows > sourceGray.rows) {
        return {false, "Template image must not be larger than the source image"};
    }

    const int method = matchMethodFromString(m_matchMethod);
    cv::Mat matchResult;
    cv::matchTemplate(sourceGray, templateGray, matchResult, method);

    double minVal = 0.0;
    double maxVal = 0.0;
    cv::Point minLoc;
    cv::Point maxLoc;
    cv::minMaxLoc(matchResult, &minVal, &maxVal, &minLoc, &maxLoc);

    const bool lowerIsBetter = isLowerBetterMatchMethod(method);
    const double matchScore = lowerIsBetter ? minVal : maxVal;
    const cv::Point matchLoc = lowerIsBetter ? minLoc : maxLoc;
    const bool matched = lowerIsBetter ? (matchScore <= m_threshold) : (matchScore >= m_threshold);

    cv::Mat output;
    if (frame.workingMat.channels() == 1) {
        cv::cvtColor(frame.workingMat, output, cv::COLOR_GRAY2BGR);
    } else {
        output = frame.workingMat.clone();
    }

    if (matched) {
        cv::rectangle(
            output,
            cv::Rect(matchLoc, cv::Size(templateGray.cols, templateGray.rows)),
            cv::Scalar(0, 255, 0),
            2
        );
    }

    frame.metrics.insert("template_match_score", matchScore);
    frame.metrics.insert("template_match_found", matched);
    frame.annotations.insert("template_image_path", templatePath);
    frame.annotations.insert("template_match_method", m_matchMethod);
    frame.workingMat = output;
    return {};
}
