#include "platform/PlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

bool PlatformPaths::isCaseInsensitiveFileSystem()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

QString PlatformPaths::defaultMinecraftDir()
{
#ifdef Q_OS_WIN
    const QString appData = qEnvironmentVariable("APPDATA");
    if (!appData.isEmpty())
        return QDir(appData).filePath(QStringLiteral(".minecraft"));
#endif
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(QStringLiteral(".minecraft"));
}

QString PlatformPaths::chooseWritableDir(const QString& preferred, const QString& fallback)
{
    if (!preferred.isEmpty() && QFileInfo(preferred).isDir() && QFileInfo(preferred).isWritable())
        return QDir(preferred).absolutePath();

    if (!fallback.isEmpty())
    {
        QDir().mkpath(fallback);
        if (QFileInfo(fallback).isDir() && QFileInfo(fallback).isWritable())
            return QDir(fallback).absolutePath();
    }

    return QDir(preferred).absolutePath();
}

QString PlatformPaths::logDir()
{
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(chooseWritableDir(QCoreApplication::applicationDirPath(), fallback))
        .filePath(QStringLiteral("logs"));
}

QString PlatformPaths::configPath()
{
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(chooseWritableDir(QCoreApplication::applicationDirPath(), fallback))
        .filePath(QStringLiteral("config.ini"));
}
