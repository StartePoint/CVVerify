#include "core/detection/OpenCvDnnRunner.h"

#include <QHash>
#include <QMutex>

#include "infra/opencv/OpenCvImageIO.h"

namespace {

QHash<QString, cv::dnn::Net>& modelCache()
{
    static QHash<QString, cv::dnn::Net> cache;
    return cache;
}

QMutex& modelCacheMutex()
{
    static QMutex mutex;
    return mutex;
}

QString& preferredBackendId()
{
    static QString backendId = "cpu";
    return backendId;
}

void applyBackendPreference(cv::dnn::Net& net)
{
    const QString backendId = preferredBackendId().trimmed().toLower();
    if (backendId == "opencl") {
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
        return;
    }
#if CV_VERSION_MAJOR >= 4
    if (backendId == "cuda") {
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        return;
    }
#endif
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

}

namespace OpenCvDnnRunner {

void clearModelCache()
{
    QMutexLocker locker(&modelCacheMutex());
    modelCache().clear();
}

void setPreferredBackend(const QString& backendId)
{
    QMutexLocker locker(&modelCacheMutex());
    preferredBackendId() = backendId;
    modelCache().clear();
}

DnnRunResult runSingleOutputModel(const QString& modelPath, const cv::Mat& blob)
{
    DnnRunResult result;

    try {
        cv::dnn::Net net;
        {
            QMutexLocker locker(&modelCacheMutex());
            auto& cache = modelCache();
            const auto it = cache.find(modelPath);
            if (it == cache.end()) {
                net = cv::dnn::readNet(OpenCvImageIO::toOpenCvFilePath(modelPath));
                if (net.empty()) {
                    result.errorMessage = QString("Failed to load model: %1").arg(modelPath);
                    return result;
                }
                applyBackendPreference(net);
                cache.insert(modelPath, net);
            } else {
                net = it.value();
            }
        }

        applyBackendPreference(net);
        net.setInput(blob);
        result.outputs.append(net.forward());
        result.success = true;
    } catch (const cv::Exception& ex) {
        result.errorMessage = QString::fromStdString(ex.what());
    }

    return result;
}

}
