#ifndef VERSIONLISTMODEL_H
#define VERSIONLISTMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include <QString>
#include <QVector>

#include "core/VersionInfo.h"

class VersionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        DisplayIdRole,
        TypeRole,
        ReleaseTimeRole,
        GroupRole,
        GroupNameRole,
        HeaderRole,
        IconRole,
        LoreRole,
        SelectedRole,
        ExpandedRole,
        CountRole
    };

    explicit VersionListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setVersions(const QVector<VersionInfo>& versions);
    void setGroupExpanded(VersionGroup group, bool expanded);
    Q_INVOKABLE void toggleGroup(int group);
    void setSelectedId(const QString& id);
    QString selectedId() const;

private:
    struct Row
    {
        bool header = false;
        VersionGroup group = VersionGroup::Release;
        int versionIndex = -1;
        int count = 0;
    };

    void rebuild();

    QVector<VersionInfo> m_versions;
    QVector<Row> m_rows;
    QSet<int> m_expandedGroups;
    QString m_selectedId;
};

#endif // VERSIONLISTMODEL_H
