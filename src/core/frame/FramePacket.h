#pragma once

#include <QString>
#include <QVariantMap>

#include <opencv2/core/mat.hpp>

struct FramePacket
{
    qint64 frameId = -1;
    qint64 timestampMs = 0;
    QString sourceId;
    cv::Mat originalMat;
    cv::Mat workingMat;
    QVariantMap artifacts;
    QVariantMap tensorOutputs;
    QVariantMap annotations;
    QVariantMap metrics;
    QVariantMap debugTrace;

    bool hasOriginalFrame() const
    {
        return !originalMat.empty();
    }

    bool hasWorkingFrame() const
    {
        return !workingMat.empty();
    }
};
