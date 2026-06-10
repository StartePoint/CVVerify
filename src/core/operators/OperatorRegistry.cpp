#include "core/operators/OperatorRegistry.h"

void OperatorRegistry::registerOperator(const OperatorDescriptor& descriptor)
{
    m_descriptors.insert(descriptor.id, descriptor);
}

QList<OperatorDescriptor> OperatorRegistry::allOperators() const
{
    return m_descriptors.values();
}

std::shared_ptr<IPipelineStep> OperatorRegistry::create(const QString& id) const
{
    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end()) {
        return {};
    }

    return it->create();
}
