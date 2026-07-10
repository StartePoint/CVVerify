#pragma once

#include <QList>
#include <QString>

#include "core/detection/DetectionModelDescriptor.h"

struct RegisteredModelPackage
{
    QString packageDir;
    QString modelPath;
    DetectionModelDescriptor descriptor;
    QStringList labels;
};

class ModelRegistry
{
public:
    bool registerPackage(const RegisteredModelPackage& package);
    bool removePackage(const QString& modelName);
    QList<RegisteredModelPackage> allPackages() const;
    bool findByName(const QString& modelName, RegisteredModelPackage* package) const;
    QString activeModelName() const;
    void setActiveModelName(const QString& modelName);

private:
    QList<RegisteredModelPackage> m_packages;
    QString m_activeModelName;
};
