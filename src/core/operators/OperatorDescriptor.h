#pragma once

#include <functional>
#include <memory>

#include "core/pipeline/IPipelineStep.h"

enum class OperatorCapabilityType
{
    BasicSingleIO,
    AdvancedSingleIO,
    InteractiveSingleInput,
    MultiResultOrMultiInput,
};

enum class OperatorInteractionType
{
    None,
    PointsOnSource,
    RectOnSource,
    SeedPointsOnSource,
    SecondImageRequired,
    MaskEditOnSource,
};

enum class OperatorResultType
{
    ImageOnly,
    ImageWithStats,
    ChartOnly,
    ImageAndChart,
    ImageAndKeypoints,
    ImageAndMatches,
    ImageAndMask,
};

struct OperatorDescriptor
{
    QString id;
    QString displayName;
    QString category;
    OperatorCapabilityType capabilityType = OperatorCapabilityType::BasicSingleIO;
    OperatorInteractionType interactionType = OperatorInteractionType::None;
    OperatorResultType resultType = OperatorResultType::ImageOnly;
    std::function<std::shared_ptr<IPipelineStep>()> create;
};
