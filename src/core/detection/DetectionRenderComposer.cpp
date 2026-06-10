#include "core/detection/DetectionRenderComposer.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

namespace DetectionRenderComposer {

void drawDetections(cv::Mat& image, const DetectionFrameResult& result)
{
    if (image.empty()) {
        return;
    }

    for (const DetectionBox& box : result.boxes) {
        const cv::Rect rect(
            static_cast<int>(box.box.x()),
            static_cast<int>(box.box.y()),
            static_cast<int>(box.box.width()),
            static_cast<int>(box.box.height())
        );

        cv::rectangle(image, rect, cv::Scalar(0, 255, 0), 2);

        const QString labelText = QString("%1 %2").arg(box.label).arg(box.score, 0, 'f', 2);
        cv::putText(
            image,
            labelText.toStdString(),
            cv::Point(rect.x, std::max(12, rect.y - 4)),
            cv::FONT_HERSHEY_SIMPLEX,
            0.45,
            cv::Scalar(0, 255, 0),
            1
        );
    }
}

}
