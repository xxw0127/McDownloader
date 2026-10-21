#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include "core/VersionInfo.h"

class VersionListModel;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QAbstractItemModel* versionModel READ versionModel CONSTANT)
    Q_PROPERTY(QVariantMap latestRelease READ latestRelease NOTIFY latestChanged)
    Q_PROPERTY(QVariantMap latestSnapshot READ latestSnapshot NOTIFY latestChanged)
    Q_PROPERTY(QString selectedVersion READ selectedVersion WRITE setSelectedVersion NOTIFY selectedVersionChanged)

public:
    explicit AppController(QObject* parent = nullptr);

    QString appName() const;
    QAbstractItemModel* versionModel();
    QVariantMap latestRelease() const;
    QVariantMap latestSnapshot() const;
    QString selectedVersion() const;
    void setSelectedVersion(const QString& versionId);

    Q_INVOKABLE void loadMockVersions();

signals:
    void latestChanged();
    void selectedVersionChanged();

private:
    void setVersions(const QVector<VersionInfo>& versions);
    QVariantMap versionToMap(const VersionInfo& version) const;
    VersionInfo latestOfGroup(VersionGroup group) const;

    VersionListModel* m_versionModel = nullptr;
    QVector<VersionInfo> m_versions;
    QString m_selectedVersion;
};

#endif // APPCONTROLLER_H
