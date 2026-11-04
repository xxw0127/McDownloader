#ifndef PATHS_H
#define PATHS_H

#include <QString>

class Paths
{
public:
    static bool isSafeRelativePath(const QString& path);
    static bool isSafeVersionId(const QString& id);
    static QString sanitizeInstanceName(const QString& name);
    static bool isWithin(const QString& root, const QString& candidate);
    static QString joinWithin(const QString& root, const QString& relative, bool* ok);
};

#endif // PATHS_H
