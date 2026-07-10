#pragma once

#include <QString>
#include <QVector>

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

struct DnnRunResult
{
    bool success = false;
    QString errorMessage;
    QVector<cv::Mat> outputs;
};

namespace OpenCvDnnRunner {

void clearModelCache();
void setPreferredBackend(const QString& backendId);
DnnRunResult runSingleOutputModel(const QString& modelPath, const cv::Mat& blob);

}
