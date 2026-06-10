#pragma once

#include <QString>

#include "core/detection/DetectionModelDescriptor.h"

namespace ModelPackageLoader {

bool saveDescriptor(const DetectionModelDescriptor& descriptor, const QString& packageDir, QString* errorMessage = nullptr);
DetectionModelLoadResult loadDescriptor(const QString& packageDir);

}
