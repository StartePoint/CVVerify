#pragma once

#include <QString>
#include <QStringList>

struct LabelLoadResult
{
    bool success = false;
    QString errorMessage;
    QStringList labels;
};

namespace LabelProvider {

LabelLoadResult loadLabels(const QString& filePath);

}
