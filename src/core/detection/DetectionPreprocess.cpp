#include "core/detection/DetectionPreprocess.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

namespace DetectionPreprocess {

LetterboxResult letterbox(const cv::Mat& source, const DetectionInputConfig& config)
{
    LetterboxResult result;
    if (source.empty()) {
        return result;
    }

    const cv::Size targetSize(config.width, config.height);
    if (!config.letterbox) {
        cv::resize(source, result.image, targetSize);
        result.scale = 1.0f;
        return result;
    }

    const float scaleW = static_cast<float>(targetSize.width) / static_cast<float>(source.cols);
    const float scaleH = static_cast<float>(targetSize.height) / static_cast<float>(source.rows);
    result.scale = config.keepRatio ? std::min(scaleW, scaleH) : scaleW;

    const int newWidth = std::max(1, static_cast<int>(std::round(source.cols * result.scale)));
    const int newHeight = std::max(1, static_cast<int>(std::round(source.rows * result.scale)));

    cv::Mat resized;
    cv::resize(source, resized, cv::Size(newWidth, newHeight));

    result.padLeft = (targetSize.width - newWidth) / 2;
    result.padTop = (targetSize.height - newHeight) / 2;
    const int padRight = targetSize.width - newWidth - result.padLeft;
    const int padBottom = targetSize.height - newHeight - result.padTop;

    cv::copyMakeBorder(
        resized,
        result.image,
        result.padTop,
        padBottom,
        result.padLeft,
        padRight,
        cv::BORDER_CONSTANT,
        cv::Scalar(114, 114, 114)
    );
    return result;
}

cv::Mat buildBlob(const cv::Mat& source, const DetectionInputConfig& config, LetterboxResult* letterboxMeta)
{
    const LetterboxResult prepared = letterbox(source, config);
    if (letterboxMeta) {
        *letterboxMeta = prepared;
    }

    const bool swapRB = config.colorOrder.compare("RGB", Qt::CaseInsensitive) == 0;
    return cv::dnn::blobFromImage(
        prepared.image,
        config.scale,
        cv::Size(config.width, config.height),
        cv::Scalar(),
        swapRB,
        false
    );
}

}
