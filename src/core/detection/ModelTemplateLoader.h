#pragma once

#include <QString>
#include <QStringList>

#include "core/detection/DetectionModelDescriptor.h"
#include "core/detection/ModelImportService.h"

namespace ModelTemplateLoader {

QStringList listTemplates();
bool loadTemplate(const QString& templateName, DetectionModelDescriptor* descriptor, QString* errorMessage = nullptr);
void applyTemplateDefaults(DetectionModelDescriptor& target, const DetectionModelDescriptor& templateDescriptor);

ModelSourceType sourceTypeFromTaskType(const QString& taskType);

}
