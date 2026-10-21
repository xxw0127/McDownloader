#include "models/VersionListModel.h"

#include "core/VersionGrouping.h"

VersionListModel::VersionListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int VersionListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows.size();
}

QVariant VersionListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return QVariant();

    const Row& row = m_rows.at(index.row());
    const QString groupName = VersionGrouping::displayName(row.group);

    if (row.header)
    {
        switch (role)
        {
        case DisplayIdRole:
        case GroupNameRole:
            return groupName;
        case GroupRole:
            return static_cast<int>(row.group);
        case HeaderRole:
            return true;
        case ExpandedRole:
            return m_expandedGroups.contains(static_cast<int>(row.group));
        case CountRole:
            return row.count;
        case SelectedRole:
            return false;
        default:
            return QVariant();
        }
    }

    if (row.versionIndex < 0 || row.versionIndex >= m_versions.size())
        return QVariant();

    const VersionInfo& version = m_versions.at(row.versionIndex);
    switch (role)
    {
    case IdRole:
        return version.id;
    case DisplayIdRole:
        return version.displayId;
    case TypeRole:
        return version.type;
    case ReleaseTimeRole:
        return VersionGrouping::formatReleaseTime(version.releaseTime);
    case GroupRole:
        return static_cast<int>(row.group);
    case GroupNameRole:
        return groupName;
    case HeaderRole:
        return false;
    case IconRole:
        return VersionGrouping::iconPath(row.group);
    case LoreRole:
        return version.lore;
    case SelectedRole:
        return version.id == m_selectedId;
    case ExpandedRole:
        return false;
    case CountRole:
        return 0;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> VersionListModel::roleNames() const
{
    return {
        {IdRole, QByteArrayLiteral("versionId")},
        {DisplayIdRole, QByteArrayLiteral("displayId")},
        {TypeRole, QByteArrayLiteral("type")},
        {ReleaseTimeRole, QByteArrayLiteral("releaseTime")},
        {GroupRole, QByteArrayLiteral("group")},
        {GroupNameRole, QByteArrayLiteral("groupName")},
        {HeaderRole, QByteArrayLiteral("header")},
        {IconRole, QByteArrayLiteral("icon")},
        {LoreRole, QByteArrayLiteral("lore")},
        {SelectedRole, QByteArrayLiteral("selected")},
        {ExpandedRole, QByteArrayLiteral("expanded")},
        {CountRole, QByteArrayLiteral("count")}
    };
}

void VersionListModel::setVersions(const QVector<VersionInfo>& versions)
{
    m_versions = versions;
    rebuild();
}

void VersionListModel::setGroupExpanded(VersionGroup group, bool expanded)
{
    const int key = static_cast<int>(group);
    const bool currentlyExpanded = m_expandedGroups.contains(key);
    if (currentlyExpanded == expanded)
        return;

    if (expanded)
        m_expandedGroups.insert(key);
    else
        m_expandedGroups.remove(key);

    rebuild();
}

void VersionListModel::toggleGroup(int group)
{
    setGroupExpanded(static_cast<VersionGroup>(group), !m_expandedGroups.contains(group));
}

void VersionListModel::setSelectedId(const QString& id)
{
    if (m_selectedId == id)
        return;

    m_selectedId = id;
    if (!m_rows.isEmpty())
        emit dataChanged(index(0, 0), index(m_rows.size() - 1, 0), {SelectedRole});
}

QString VersionListModel::selectedId() const
{
    return m_selectedId;
}

void VersionListModel::rebuild()
{
    beginResetModel();
    m_rows.clear();

    for (const VersionGroup group : VersionGrouping::order())
    {
        Row header;
        header.header = true;
        header.group = group;
        header.count = 0;

        QVector<int> indices;
        for (int i = 0; i < m_versions.size(); ++i)
        {
            if (VersionGrouping::classify(m_versions.at(i)) == group)
            {
                indices.append(i);
                ++header.count;
            }
        }
        m_rows.append(header);

        if (m_expandedGroups.contains(static_cast<int>(group)))
        {
            for (const int versionIndex : indices)
            {
                Row row;
                row.header = false;
                row.group = group;
                row.versionIndex = versionIndex;
                m_rows.append(row);
            }
        }
    }

    endResetModel();
}
