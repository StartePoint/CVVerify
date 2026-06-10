#pragma once

#include <QString>
#include <QVector>

#include <opencv2/core.hpp>

struct DnnRunResult
{
    bool success = false;
    QString errorMessage;
    QVector<cv::Mat> outputs;
};

namespace OpenCvDnnRunner {

DnnRunResult runSingleOutputModel(const QString& modelPath, const cv::Mat& blob);

}
