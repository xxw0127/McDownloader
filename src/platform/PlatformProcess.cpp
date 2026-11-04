#include "platform/PlatformProcess.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

QString PlatformProcess::javaExecutableName()
{
#ifdef Q_OS_WIN
    return QStringLiteral("java.exe");
#else
    return QStringLiteral("java");
#endif
}

QString PlatformProcess::minecraftOsName()
{
#ifdef Q_OS_WIN
    return QStringLiteral("windows");
#else
    return QStringLiteral("linux");
#endif
}

QString PlatformProcess::minecraftArch()
{
    return sizeof(void*) == 8 ? QStringLiteral("x64") : QStringLiteral("x86");
}

QStringList PlatformProcess::commonJavaRoots()
{
#ifdef Q_OS_WIN
    return {
        QStringLiteral("C:/Program Files/Java"),
        QStringLiteral("C:/Program Files/Eclipse Adoptium"),
        QStringLiteral("C:/Program Files/Microsoft"),
        QStringLiteral("C:/Program Files/BellSoft"),
        QStringLiteral("D:/Java"),
        QStringLiteral("D:/Program Files/Java")
    };
#else
    return {
        QStringLiteral("/usr/lib/jvm"),
        QStringLiteral("/usr/java")
    };
#endif
}

QString PlatformProcess::cmdExecutable()
{
#ifdef Q_OS_WIN
    return QStringLiteral("cmd.exe");
#else
    return QStringLiteral("/bin/sh");
#endif
}

bool PlatformProcess::createDirectoryLink(const QString& link, const QString& target, QString* error)
{
    if (link.isEmpty() || target.isEmpty())
    {
        if (error != nullptr)
            *error = QStringLiteral("目录链接参数为空");
        return false;
    }

#ifdef Q_OS_WIN
    const QFileInfo linkInfo(link);
    if (linkInfo.exists() || linkInfo.isSymLink())
        QProcess::execute(QStringLiteral("cmd.exe"),
                          {QStringLiteral("/c"), QStringLiteral("rmdir"),
                           QDir::toNativeSeparators(link)});
    const int code = QProcess::execute(QStringLiteral("cmd.exe"),
                                       {QStringLiteral("/c"), QStringLiteral("mklink"),
                                        QStringLiteral("/J"), QDir::toNativeSeparators(link),
                                        QDir::toNativeSeparators(target)});
    if (code != 0)
    {
        if (error != nullptr)
            *error = QStringLiteral("无法创建目录链接：%1").arg(link);
        return false;
    }
#else
    if (QFileInfo::exists(link))
        QDir(link).removeRecursively();
    if (!QFile::link(target, link))
    {
        if (error != nullptr)
            *error = QStringLiteral("无法创建目录链接：%1").arg(link);
        return false;
    }
#endif
    return true;
}
