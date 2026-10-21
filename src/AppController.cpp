#include "AppController.h"

#include <QFile>

#include "core/Logger.h"
#include "core/VersionGrouping.h"
#include "core/VersionManifest.h"
#include "models/VersionListModel.h"

AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_versionModel(new VersionListModel(this))
{
}

QString AppController::appName() const
{
    return QStringLiteral("PCL2 下载器");
}

QAbstractItemModel* AppController::versionModel()
{
    return m_versionModel;
}

QVariantMap AppController::latestRelease() const
{
    return versionToMap(latestOfGroup(VersionGroup::Release));
}

QVariantMap AppController::latestSnapshot() const
{
    return versionToMap(latestOfGroup(VersionGroup::Snapshot));
}

QString AppController::selectedVersion() const
{
    return m_selectedVersion;
}

void AppController::setSelectedVersion(const QString& versionId)
{
    if (m_selectedVersion == versionId)
        return;

    m_selectedVersion = versionId;
    m_versionModel->setSelectedId(versionId);
    emit selectedVersionChanged();
    emit latestChanged();
}

void AppController::loadMockVersions()
{
    QFile file(QStringLiteral(":/fixtures/manifest_small.json"));
    if (!file.open(QIODevice::ReadOnly))
    {
        Logger::error(QStringLiteral("AppController"), QStringLiteral("无法打开内置版本 fixture"));
        return;
    }

    QString error;
    const QVector<VersionInfo> versions = VersionManifest::parse(file.readAll(), &error);
    if (!error.isEmpty())
    {
        Logger::error(QStringLiteral("AppController"), error);
        return;
    }

    setVersions(versions);
    Logger::info(QStringLiteral("AppController"), QStringLiteral("加载 mock 版本 %1 个").arg(versions.size()));
}

void AppController::setVersions(const QVector<VersionInfo>& versions)
{
    m_versions = versions;
    m_versionModel->setVersions(versions);
    emit latestChanged();
}

QVariantMap AppController::versionToMap(const VersionInfo& version) const
{
    if (version.id.isEmpty())
        return {};

    const VersionGroup group = VersionGrouping::classify(version);
    QVariantMap map;
    map.insert(QStringLiteral("id"), version.id);
    map.insert(QStringLiteral("displayId"), version.displayId);
    map.insert(QStringLiteral("type"), version.type);
    map.insert(QStringLiteral("releaseTime"), VersionGrouping::formatReleaseTime(version.releaseTime));
    map.insert(QStringLiteral("icon"), VersionGrouping::iconPath(group));
    map.insert(QStringLiteral("lore"), version.lore);
    map.insert(QStringLiteral("selected"), version.id == m_selectedVersion);
    return map;
}

VersionInfo AppController::latestOfGroup(VersionGroup group) const
{
    VersionInfo result;
    bool found = false;
    for (const VersionInfo& version : m_versions)
    {
        if (VersionGrouping::classify(version) != group)
            continue;
        if (!found || version.releaseTime > result.releaseTime)
        {
            result = version;
            found = true;
        }
    }
    return result;
}
