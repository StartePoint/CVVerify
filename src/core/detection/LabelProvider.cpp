#include "core/detection/LabelProvider.h"

#include <QFile>
#include <QTextStream>

namespace LabelProvider {

LabelLoadResult loadLabels(const QString& filePath)
{
    LabelLoadResult result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QString("Failed to open labels file: %1").arg(filePath);
        return result;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (!line.isEmpty()) {
            result.labels.append(line);
        }
    }

    if (result.labels.isEmpty()) {
        result.errorMessage = QString("Labels file is empty: %1").arg(filePath);
        return result;
    }

    result.success = true;
    return result;
}

}
