#include "core/models/ModelRegistry.h"

bool ModelRegistry::registerPackage(const RegisteredModelPackage& package)
{
    for (int index = 0; index < m_packages.size(); ++index) {
        if (m_packages[index].descriptor.modelName == package.descriptor.modelName) {
            m_packages[index] = package;
            return true;
        }
    }

    m_packages.append(package);
    if (m_activeModelName.isEmpty()) {
        m_activeModelName = package.descriptor.modelName;
    }
    return true;
}

bool ModelRegistry::removePackage(const QString& modelName)
{
    for (int index = 0; index < m_packages.size(); ++index) {
        if (m_packages[index].descriptor.modelName == modelName) {
            m_packages.removeAt(index);
            if (m_activeModelName == modelName) {
                m_activeModelName = m_packages.isEmpty() ? QString() : m_packages.first().descriptor.modelName;
            }
            return true;
        }
    }
    return false;
}

QList<RegisteredModelPackage> ModelRegistry::allPackages() const
{
    return m_packages;
}

bool ModelRegistry::findByName(const QString& modelName, RegisteredModelPackage* package) const
{
    if (!package) {
        return false;
    }

    for (const RegisteredModelPackage& entry : m_packages) {
        if (entry.descriptor.modelName == modelName) {
            *package = entry;
            return true;
        }
    }
    return false;
}

QString ModelRegistry::activeModelName() const
{
    return m_activeModelName;
}

void ModelRegistry::setActiveModelName(const QString& modelName)
{
    m_activeModelName = modelName;
}
