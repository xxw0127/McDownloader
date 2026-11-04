#ifndef PLATFORMPROCESS_H
#define PLATFORMPROCESS_H

#include <QString>
#include <QStringList>

class PlatformProcess
{
public:
    static QString javaExecutableName();
    static QStringList commonJavaRoots();
    static QString cmdExecutable();
    static bool createDirectoryLink(const QString& link, const QString& target, QString* error);
};

#endif // PLATFORMPROCESS_H
