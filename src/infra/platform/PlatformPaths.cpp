#include "infra/platform/PlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace PlatformPaths {

namespace {

QStringList resourceSearchRoots()
{
    QStringList roots;

    const auto appendWalkUp = [&roots](const QString& startPath) {
        if (startPath.isEmpty()) {
            return;
        }

        QDir dir(startPath);
        for (int depth = 0; depth < 8; ++depth) {
            const QString absolute = dir.absolutePath();
            if (!absolute.isEmpty() && !roots.contains(absolute)) {
                roots.append(absolute);
            }
            if (!dir.cdUp()) {
                break;
            }
        }
    };

    appendWalkUp(QCoreApplication::applicationDirPath());
    appendWalkUp(QDir::currentPath());

    return roots;
}

} // namespace

QString applicationDirPath()
{
    return QCoreApplication::applicationDirPath();
}

QString configDirectory()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(path);
    return path;
}

QString modelsDirectory()
{
    const QString path = QDir(configDirectory()).filePath("models");
    QDir().mkpath(path);
    return path;
}

QString exportsDirectory()
{
    const QString path = QDir(configDirectory()).filePath("exports");
    QDir().mkpath(path);
    return path;
}

QString presetsDirectory()
{
    const QString path = QDir(configDirectory()).filePath("presets");
    QDir().mkpath(path);
    return path;
}

QString resolveResourcePath(const QString& relativePath)
{
    const QString trimmedPath = relativePath.trimmed();
    if (trimmedPath.isEmpty()) {
        return QString();
    }

    for (const QString& rootPath : resourceSearchRoots()) {
        const QString candidate = QDir(rootPath).filePath(trimmedPath);
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return QDir(applicationDirPath()).filePath(trimmedPath);
}

QString resolveDocumentationPath(const QString& relativePath)
{
    const QString trimmedPath = relativePath.trimmed();
    if (trimmedPath.isEmpty()) {
        return QString();
    }

    const QString normalized = trimmedPath.startsWith("docs/")
        ? trimmedPath
        : QStringLiteral("docs/") + trimmedPath;

    for (const QString& rootPath : resourceSearchRoots()) {
        const QString candidate = QDir(rootPath).filePath(normalized);
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return QDir(QDir::currentPath()).filePath(normalized);
}

}
