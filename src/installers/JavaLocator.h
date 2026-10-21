#ifndef JAVALOCATOR_H
#define JAVALOCATOR_H

#include <QString>
#include <QVector>

struct JavaInfo
{
    QString path;
    int major = 0;
    bool is64Bit = false;
    QString versionText;
};

class JavaLocator
{
public:
    static JavaInfo parseVersionOutput(const QString& output);
    static QVector<JavaInfo> scan(const QString& minecraftDir);
    static JavaInfo select(const QVector<JavaInfo>& candidates, int requiredMajor);
    static JavaInfo selectForInstaller(const QVector<JavaInfo>& candidates, int requiredMajor);
};

#endif // JAVALOCATOR_H
