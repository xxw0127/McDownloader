#ifndef VERSIONGROUPING_H
#define VERSIONGROUPING_H

#include <QString>
#include <QVector>

#include "core/VersionInfo.h"

class VersionGrouping
{
public:
    static VersionGroup classify(const VersionInfo& version);
    static QString displayName(VersionGroup group);
    static QString iconPath(VersionGroup group);
    static QString formatReleaseTime(const QDateTime& time);
    static QVector<VersionInfo> latestForCard(const QVector<VersionInfo>& versions);
    static QVector<VersionInfo> forGroup(const QVector<VersionInfo>& versions, VersionGroup group);
    static QVector<VersionGroup> order();
};

#endif // VERSIONGROUPING_H
