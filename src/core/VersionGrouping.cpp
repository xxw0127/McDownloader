#include "core/VersionGrouping.h"

#include <QSet>

namespace
{
bool isAprilFoolsId(const QString& id)
{
    static const QSet<QString> ids = {
        QStringLiteral("2.0"),
        QStringLiteral("15w14a"),
        QStringLiteral("1.rv-pre1"),
        QStringLiteral("3d shareware v1.34"),
        QStringLiteral("20w14∞"),
        QStringLiteral("20w14infinite"),
        QStringLiteral("22w13oneblockatatime"),
        QStringLiteral("23w13a_or_b"),
        QStringLiteral("24w14potato"),
        QStringLiteral("25w14craftmine"),
        QStringLiteral("26w14a")
    };
    return ids.contains(id.toLower());
}

bool isAprilFoolsDate(const VersionInfo& version)
{
    if (!version.releaseTime.isValid())
        return false;

    // PCL2 使用 UTC+2 判断 4 月 1 日。
    const QDateTime shifted = version.releaseTime.toUTC().addSecs(2 * 60 * 60);
    return shifted.date().month() == 4 && shifted.date().day() == 1;
}

bool looksLikePseudoRelease(const QString& id)
{
    if (!id.startsWith(QLatin1String("1.")))
        return false;

    const QString lower = id.toLower();
    const QStringList excluded = {
        QStringLiteral("combat"),
        QStringLiteral("rc"),
        QStringLiteral("experimental"),
        QStringLiteral("pre")
    };
    for (const QString& keyword : excluded)
    {
        if (lower.contains(keyword))
            return false;
    }
    return true;
}
}

VersionGroup VersionGrouping::classify(const VersionInfo& version)
{
    const QString type = version.type.toLower();

    if (isAprilFoolsId(version.id) || isAprilFoolsDate(version))
        return VersionGroup::AprilFools;

    if (type == QLatin1String("release"))
        return VersionGroup::Release;
    if (type == QLatin1String("special"))
        return VersionGroup::AprilFools;
    if (type == QLatin1String("old_alpha") || type == QLatin1String("old_beta"))
        return VersionGroup::Old;

    if (type == QLatin1String("snapshot"))
    {
        if (looksLikePseudoRelease(version.id))
            return VersionGroup::Release;
        return VersionGroup::Snapshot;
    }

    return VersionGroup::Old;
}

QString VersionGrouping::displayName(VersionGroup group)
{
    switch (group)
    {
    case VersionGroup::Release:
        return QStringLiteral("正式版");
    case VersionGroup::Snapshot:
        return QStringLiteral("预览版");
    case VersionGroup::Old:
        return QStringLiteral("远古版");
    case VersionGroup::AprilFools:
        return QStringLiteral("愚人节版");
    }
    return QString();
}

QString VersionGrouping::iconPath(VersionGroup group)
{
    switch (group)
    {
    case VersionGroup::Release:
        return QStringLiteral("qrc:/images/grass.png");
    case VersionGroup::Snapshot:
        return QStringLiteral("qrc:/images/command_block.png");
    case VersionGroup::Old:
        return QStringLiteral("qrc:/images/cobblestone.png");
    case VersionGroup::AprilFools:
        return QStringLiteral("qrc:/images/gold_block.png");
    }
    return QString();
}

QString VersionGrouping::formatReleaseTime(const QDateTime& time)
{
    if (!time.isValid())
        return QString();
    return time.toLocalTime().toString(QStringLiteral("yyyy/MM/dd HH:mm"));
}

QVector<VersionInfo> VersionGrouping::latestForCard(const QVector<VersionInfo>& versions)
{
    VersionInfo release;
    VersionInfo snapshot;
    bool hasRelease = false;
    bool hasSnapshot = false;

    for (const VersionInfo& version : versions)
    {
        const VersionGroup group = classify(version);
        if (group == VersionGroup::Release)
        {
            if (!hasRelease || version.releaseTime > release.releaseTime)
            {
                release = version;
                hasRelease = true;
            }
        }
        else if (group == VersionGroup::Snapshot)
        {
            if (!hasSnapshot || version.releaseTime > snapshot.releaseTime)
            {
                snapshot = version;
                hasSnapshot = true;
            }
        }
    }

    QVector<VersionInfo> result;
    if (hasRelease)
        result.append(release);
    if (hasSnapshot && (!hasRelease || snapshot.releaseTime > release.releaseTime))
        result.append(snapshot);
    return result;
}

QVector<VersionInfo> VersionGrouping::forGroup(const QVector<VersionInfo>& versions, VersionGroup group)
{
    QVector<VersionInfo> result;
    for (const VersionInfo& version : versions)
    {
        if (classify(version) == group)
            result.append(version);
    }
    return result;
}

QVector<VersionGroup> VersionGrouping::order()
{
    return {VersionGroup::Release, VersionGroup::Snapshot, VersionGroup::Old, VersionGroup::AprilFools};
}
