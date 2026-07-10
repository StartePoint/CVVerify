#include "core/diagnostics/DnnRuntimeInfo.h"

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/dnn.hpp>

#include "core/diagnostics/DiagnosticsLog.h"

namespace DnnRuntimeInfo {

namespace {

QString backendIdFromOpenCvBackend(int backend)
{
    switch (backend) {
    case cv::dnn::DNN_BACKEND_OPENCV:
        return QStringLiteral("cpu");
    case cv::dnn::DNN_BACKEND_CUDA:
        return QStringLiteral("cuda");
    default:
        return QString();
    }
}

bool appendBackend(QStringList* backends, const QString& backendId)
{
    if (!backends || backendId.isEmpty() || backends->contains(backendId)) {
        return false;
    }
    backends->append(backendId);
    return true;
}

}

BackendInfo queryBackendInfo()
{
    BackendInfo info;
    info.opencvVersion = QString::fromStdString(CV_VERSION);
    info.availableBackends = QStringList() << QStringLiteral("cpu");

    if (cv::ocl::haveOpenCL()) {
        appendBackend(&info.availableBackends, QStringLiteral("opencl"));
    }

#if CV_VERSION_MAJOR >= 4
    try {
        const auto availablePairs = cv::dnn::getAvailableBackends();
        for (const auto& pair : availablePairs) {
            const QString backendId = backendIdFromOpenCvBackend(static_cast<int>(pair.first));
            appendBackend(&info.availableBackends, backendId);
        }
    } catch (const cv::Exception&) {
        // Fall back to compile-time defaults when dnn backend probing is unavailable.
    }
#endif

    info.preferredBackend = QStringLiteral("opencv_cpu");
    info.preferredTarget = QStringLiteral("cpu");
    return info;
}

void logBackendInfo()
{
    const BackendInfo info = queryBackendInfo();
    DiagnosticsLog::instance().info(
        "dnn",
        QString("OpenCV %1 | backends=%2 | default=%3/%4")
            .arg(info.opencvVersion, info.availableBackends.join(","), info.preferredBackend, info.preferredTarget));
}

}
