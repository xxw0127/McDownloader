#include "platform/PlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryFile>

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

namespace
{
bool canWriteDirectory(const QString& path)
{
    if (path.isEmpty())
        return false;

    QDir dir(path);
    if (!dir.exists() && !QDir().mkpath(path))
        return false;

    QTemporaryFile probe(dir.filePath(QStringLiteral("mcdownloader-write-XXXXXX")));
    if (!probe.open())
        return false;
    probe.close();
    return true;
}
}

QString PlatformPaths::chooseWritableDir(const QString& preferred, const QString& fallback)
{
    if (canWriteDirectory(preferred))
        return QDir(preferred).absolutePath();

    if (canWriteDirectory(fallback))
        return QDir(fallback).absolutePath();

    return QString();
}

QString PlatformPaths::cacheDir()
{
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString dir = chooseWritableDir(QCoreApplication::applicationDirPath(), fallback);
    if (dir.isEmpty())
        dir = QDir::homePath() + QStringLiteral("/.mcdownloader");
    dir = QDir(dir).filePath(QStringLiteral("cache"));
    QDir().mkpath(dir);
    return dir;
}

QString PlatformPaths::logDir()
{
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString dir = chooseWritableDir(QCoreApplication::applicationDirPath(), fallback);
    if (dir.isEmpty())
        dir = QDir::homePath() + QStringLiteral("/.mcdownloader");
    return QDir(dir).filePath(QStringLiteral("logs"));
}

QString PlatformPaths::configPath()
{
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString dir = chooseWritableDir(QCoreApplication::applicationDirPath(), fallback);
    if (dir.isEmpty())
        dir = QDir::homePath() + QStringLiteral("/.mcdownloader");
    return QDir(dir).filePath(QStringLiteral("config.ini"));
}
