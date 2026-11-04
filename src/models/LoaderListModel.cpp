#include "models/LoaderListModel.h"

LoaderListModel::LoaderListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int LoaderListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_versions.size();
}

QVariant LoaderListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_versions.size())
        return QVariant();

    const LoaderVersion& version = m_versions.at(index.row());
    switch (role)
    {
    case VersionRole:
        return version.version;
    case StableRole:
        return version.stable;
    case ReleasedRole:
        return version.released;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> LoaderListModel::roleNames() const
{
    return {
        {VersionRole, QByteArrayLiteral("version")},
        {StableRole, QByteArrayLiteral("stable")},
        {ReleasedRole, QByteArrayLiteral("released")}
    };
}

void LoaderListModel::setVersions(const QVector<LoaderVersion>& versions)
{
    beginResetModel();
    m_versions = versions;
    endResetModel();
}
