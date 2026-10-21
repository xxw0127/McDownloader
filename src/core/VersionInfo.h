#ifndef VERSIONINFO_H
#define VERSIONINFO_H

#include <QDateTime>
#include <QString>
#include <QUrl>

enum class VersionGroup
{
    Release,
    Snapshot,
    Old,
    AprilFools
};

struct VersionInfo
{
    QString id;
    QString type;
    QString displayId;
    QDateTime releaseTime;
    QUrl url;
    QString lore;
};

#endif // VERSIONINFO_H
