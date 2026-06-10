#pragma once

#include <memory>

#include <QHash>
#include <QList>

#include "core/operators/OperatorDescriptor.h"

class OperatorRegistry
{
public:
    void registerOperator(const OperatorDescriptor& descriptor);
    QList<OperatorDescriptor> allOperators() const;
    std::shared_ptr<IPipelineStep> create(const QString& id) const;

private:
    QHash<QString, OperatorDescriptor> m_descriptors;
};
