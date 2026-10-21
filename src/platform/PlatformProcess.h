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
};

#endif // PLATFORMPROCESS_H
