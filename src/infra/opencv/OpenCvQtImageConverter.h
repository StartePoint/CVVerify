#pragma once

#include <QImage>

#include <opencv2/core/mat.hpp>

namespace OpenCvQtImageConverter {

QImage toQImage(const cv::Mat& mat);

}
