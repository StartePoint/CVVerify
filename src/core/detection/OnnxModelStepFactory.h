#pragma once

#include <memory>

#include <QString>
#include <QStringList>

#include "core/detection/DetectionModelDescriptor.h"
#include "core/pipeline/IPipelineStep.h"

namespace OnnxModelStepFactory {

bool isDetectionTask(const QString& taskType);
bool isClassificationTask(const QString& taskType);
bool isSegmentationTask(const QString& taskType);
bool isOcrTask(const QString& taskType);
bool isKeypointTask(const QString& taskType);
bool isCustomTask(const QString& taskType);

std::shared_ptr<IPipelineStep> createStep(
    const DetectionModelDescriptor& descriptor,
    const QString& modelPath,
    const QStringList& labels);

}
