#include "core/AppSettings.h"

#include <QSettings>

#include "platform/PlatformPaths.h"

AppSettings::AppSettings(QObject* parent, const QString& iniPath)
    : QObject(parent)
{
    const QString path = iniPath.isEmpty() ? PlatformPaths::configPath() : iniPath;
    m_settings = new QSettings(path, QSettings::IniFormat, this);
}

QString AppSettings::minecraftDir() const
{
    return m_settings->value(QStringLiteral("paths/minecraftDir"),
                             PlatformPaths::defaultMinecraftDir())
        .toString();
}

void AppSettings::setMinecraftDir(const QString& path)
{
    m_settings->setValue(QStringLiteral("paths/minecraftDir"), path);
    m_settings->sync();
}

QString AppSettings::javaPath() const
{
    return m_settings->value(QStringLiteral("java/path")).toString();
}

void AppSettings::setJavaPath(const QString& path)
{
    m_settings->setValue(QStringLiteral("java/path"), path);
    m_settings->sync();
}
