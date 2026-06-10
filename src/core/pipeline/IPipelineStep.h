#pragma once

#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QVector>

#include "core/frame/FramePacket.h"

enum class StepParameterType
{
    Integer,
    Double,
    Boolean,
    String,
    Choice
};

struct StepParameterChoice
{
    QString value;
    QString label;
};

struct StepParameter
{
    QString key;
    QString displayName;
    QString group;
    StepParameterType type = StepParameterType::String;
    QVariant defaultValue;
    QVariant minimumValue;
    QVariant maximumValue;
    QVariant stepValue;
    QString visibleWhen;
    QString tooltip;
    QVector<StepParameterChoice> choices;
};

struct StepSchema
{
    QString id;
    QString displayName;
    QVector<StepParameter> parameters;
};

struct RunContext
{
    bool isPreview = true;
};

struct StepResult
{
    bool success = true;
    QString errorMessage;
};

class IPipelineStep
{
public:
    virtual ~IPipelineStep() = default;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual StepSchema schema() const = 0;
    virtual QVariantMap parameterValues() const = 0;
    virtual void setParameterValues(const QVariantMap& values) = 0;
    virtual StepResult execute(FramePacket& frame, const RunContext& context) = 0;
};
