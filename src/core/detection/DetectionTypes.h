#pragma once

#include <QRectF>
#include <QString>
#include <QVariantMap>
#include <QVector>

struct DetectionBox
{
    int classId = -1;
    QString label;
    float score = 0.0f;
    QRectF box;
};

struct DetectionFrameResult
{
    qint64 frameId = -1;
    qint64 timestampMs = 0;
    QString sourceId;
    QVector<DetectionBox> boxes;
    QVariantMap runtimeMeta;
};
