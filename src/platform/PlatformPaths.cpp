#include "platform/PlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

QString PlatformPaths::defaultMinecraftDir()
{
#ifdef Q_OS_WIN
    const QString appData = qEnvironmentVariable("APPDATA");
    if (!appData.isEmpty())
        return QDir(appData).filePath(QStringLiteral(".minecraft"));
#endif
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(QStringLiteral(".minecraft"));
}

QString PlatformPaths::logDir()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("logs"));
}

QString PlatformPaths::configPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config.ini"));
}
