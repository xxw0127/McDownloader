#ifndef LOADERLISTMODEL_H
#define LOADERLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "core/LoaderKind.h"

class LoaderListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        VersionRole = Qt::UserRole + 1,
        StableRole,
        ReleasedRole
    };

    explicit LoaderListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setVersions(const QVector<LoaderVersion>& versions);

private:
    QVector<LoaderVersion> m_versions;
};

#endif // LOADERLISTMODEL_H
