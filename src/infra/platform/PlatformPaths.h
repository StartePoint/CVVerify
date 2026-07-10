#pragma once

#include <QString>

namespace PlatformPaths {

QString applicationDirPath();
QString configDirectory();
QString modelsDirectory();
QString exportsDirectory();
QString presetsDirectory();
QString resolveResourcePath(const QString& relativePath);
QString resolveDocumentationPath(const QString& relativePath);

}
