#ifndef VERSIONMANIFEST_H
#define VERSIONMANIFEST_H

#include <QByteArray>
#include <QString>
#include <QVector>

#include "core/VersionInfo.h"

class VersionManifest
{
public:
    static QVector<VersionInfo> parse(const QByteArray& json, QString* error);
    static QVector<VersionInfo> merge(const QVector<VersionInfo>& primary,
                                      const QVector<VersionInfo>& secondary);
};

#endif // VERSIONMANIFEST_H
