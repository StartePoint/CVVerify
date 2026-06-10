#include "core/operators/builtin/GrabCutOperator.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

namespace {
bool loadGrabCutRectFromArtifacts(const QVariantMap& artifacts, cv::Rect* rect)
{
    if (!rect) {
        return false;
    }

    const QVariantMap rectMap = artifacts.value("grabcut_rect").toMap();
    if (!rectMap.contains("x") || !rectMap.contains("y") || !rectMap.contains("width") || !rectMap.contains("height")) {
        return false;
    }

    const int x = qRound(rectMap.value("x").toDouble());
    const int y = qRound(rectMap.value("y").toDouble());
    const int width = qRound(rectMap.value("width").toDouble());
    const int height = qRound(rectMap.value("height").toDouble());
    if (width <= 0 || height <= 0) {
        return false;
    }

    *rect = cv::Rect(x, y, width, height);
    return true;
}

cv::Mat ensureColorForGrabCut(const cv::Mat& source)
{
    if (source.channels() == 3) {
        return source.clone();
    }

    cv::Mat color;
    cv::cvtColor(source, color, cv::COLOR_GRAY2BGR);
    return color;
}

cv::Mat createGrabCutInitMask(const cv::Size& size, const cv::Rect& rect)
{
    cv::Mat mask(size, CV_8UC1, cv::Scalar(cv::GC_BGD));
    mask(rect).setTo(cv::Scalar(cv::GC_PR_FGD));

    if (rect.width > 4 && rect.height > 4) {
        const int insetX = std::max(1, rect.width / 6);
        const int insetY = std::max(1, rect.height / 6);
        const cv::Rect definiteRect(
            rect.x + insetX,
            rect.y + insetY,
            std::max(1, rect.width - insetX * 2),
            std::max(1, rect.height - insetY * 2)
        );
        mask(definiteRect).setTo(cv::Scalar(cv::GC_FGD));
    }

    return mask;
}
}

QString GrabCutOperator::id() const
{
    return "builtin.grabcut";
}

QString GrabCutOperator::displayName() const
{
    return "GrabCut";
}

StepSchema GrabCutOperator::schema() const
{
    return {
        "builtin.grabcut",
        "GrabCut",
        {
            {"initMode", "Init Mode", "交互设置", StepParameterType::Choice, "rect", {}, {}, {}, {}, "Mask mode derives a probable-foreground mask from the selected rectangle", {
                {"rect", "Rectangle"},
                {"mask", "Mask"},
            }},
            {"iterCount", "Iteration Count", "分割设置", StepParameterType::Integer, 5, 1, 20, 1, {}, {}, {}},
            {"outputMode", "Output Mode", "输出设置", StepParameterType::Choice, "foreground_rgba", {}, {}, {}, {}, {}, {
                {"foreground_rgba", "Foreground RGBA"},
                {"mask", "Mask"},
                {"overlay", "Overlay"},
            }},
            {"featherRadius", "Feather Radius", "输出设置", StepParameterType::Integer, 0, 0, 50, 1, "outputMode=foreground_rgba", {}, {}},
        }
    };
}

QVariantMap GrabCutOperator::parameterValues() const
{
    return {
        {"initMode", m_initMode},
        {"iterCount", m_iterCount},
        {"outputMode", m_outputMode},
        {"featherRadius", m_featherRadius},
    };
}

void GrabCutOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("initMode")) m_initMode = values.value("initMode").toString();
    if (values.contains("iterCount")) m_iterCount = values.value("iterCount").toInt();
    if (values.contains("outputMode")) m_outputMode = values.value("outputMode").toString();
    if (values.contains("featherRadius")) m_featherRadius = values.value("featherRadius").toInt();
}

StepResult GrabCutOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Rect rect;
    if (!loadGrabCutRectFromArtifacts(frame.artifacts, &rect)) {
        return {false, "GrabCut requires a rectangle in frame.artifacts[grabcut_rect]"};
    }

    cv::Mat color = ensureColorForGrabCut(frame.workingMat);
    rect &= cv::Rect(0, 0, color.cols, color.rows);
    if (rect.width <= 0 || rect.height <= 0) {
        return {false, "GrabCut rectangle is outside image bounds"};
    }

    cv::Mat mask;
    cv::Mat bgdModel;
    cv::Mat fgdModel;
    const int iterCount = std::max(1, m_iterCount);

    if (m_initMode == "mask") {
        mask = createGrabCutInitMask(color.size(), rect);
        cv::grabCut(color, mask, rect, bgdModel, fgdModel, iterCount, cv::GC_INIT_WITH_MASK);
    } else {
        mask = cv::Mat(color.size(), CV_8UC1, cv::Scalar(cv::GC_BGD));
        cv::grabCut(color, mask, rect, bgdModel, fgdModel, iterCount, cv::GC_INIT_WITH_RECT);
    }

    cv::Mat foregroundMask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
    foregroundMask.convertTo(foregroundMask, CV_8UC1, 255.0);

    if (m_outputMode == "mask") {
        frame.workingMat = foregroundMask;
        return {};
    }

    if (m_outputMode == "overlay") {
        cv::Mat overlay = color.clone();
        for (int row = 0; row < overlay.rows; ++row) {
            const uchar* maskPtr = foregroundMask.ptr<uchar>(row);
            cv::Vec3b* pixelPtr = overlay.ptr<cv::Vec3b>(row);
            for (int col = 0; col < overlay.cols; ++col) {
                if (maskPtr[col] == 0) {
                    pixelPtr[col][0] = static_cast<uchar>(pixelPtr[col][0] * 0.25);
                    pixelPtr[col][1] = static_cast<uchar>(pixelPtr[col][1] * 0.25);
                    pixelPtr[col][2] = static_cast<uchar>(pixelPtr[col][2] * 0.25);
                }
            }
        }
        frame.workingMat = overlay;
        return {};
    }

    cv::Mat alpha = foregroundMask.clone();
    if (m_featherRadius > 0) {
        cv::GaussianBlur(alpha, alpha, cv::Size(0, 0), std::max(1, m_featherRadius));
    }

    std::vector<cv::Mat> channels;
    cv::split(color, channels);
    channels.push_back(alpha);
    cv::Mat bgra;
    cv::merge(channels, bgra);
    frame.workingMat = bgra;
    return {};
}
