#include "platform/PlatformProcess.h"

QString PlatformProcess::javaExecutableName()
{
#ifdef Q_OS_WIN
    return QStringLiteral("java.exe");
#else
    return QStringLiteral("java");
#endif
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
