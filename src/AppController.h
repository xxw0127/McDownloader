#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVector>

#include "core/LoaderKind.h"
#include "core/VersionInfo.h"
#include "installers/InstallController.h"

class AppSettings;
class JsonFetcher;
class LoaderListModel;
class VersionListModel;
class QThread;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QAbstractItemModel* versionModel READ versionModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel* loaderModel READ loaderModel CONSTANT)
    Q_PROPERTY(QVariantMap latestRelease READ latestRelease NOTIFY latestChanged)
    Q_PROPERTY(QVariantMap latestSnapshot READ latestSnapshot NOTIFY latestChanged)
    Q_PROPERTY(QString selectedVersion READ selectedVersion WRITE setSelectedVersion NOTIFY selectedVersionChanged)
    Q_PROPERTY(QString minecraftDir READ minecraftDir WRITE setMinecraftDir NOTIFY minecraftDirChanged)
    Q_PROPERTY(QString javaPath READ javaPath WRITE setJavaPath NOTIFY javaPathChanged)
    Q_PROPERTY(QString loaderKind READ loaderKind WRITE setLoaderKind NOTIFY loaderKindChanged)
    Q_PROPERTY(QString loaderVersion READ loaderVersion WRITE setLoaderVersion NOTIFY loaderVersionChanged)
    Q_PROPERTY(bool loaderLoading READ isLoaderLoading NOTIFY loaderLoadingChanged)
    Q_PROPERTY(bool installing READ isInstalling NOTIFY installingChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QVariantMap selectedInfo READ selectedInfo NOTIFY selectedVersionChanged)
    Q_PROPERTY(int versionCount READ versionCount NOTIFY latestChanged)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    QString appName() const;
    QAbstractItemModel* versionModel();
    QAbstractItemModel* loaderModel();
    QVariantMap latestRelease() const;
    QVariantMap latestSnapshot() const;
    QString selectedVersion() const;
    void setSelectedVersion(const QString& versionId);
    QString minecraftDir() const;
    void setMinecraftDir(const QString& path);
    QString javaPath() const;
    void setJavaPath(const QString& path);
    QString loaderKind() const;
    void setLoaderKind(const QString& kind);
    QString loaderVersion() const;
    void setLoaderVersion(const QString& version);
    bool isLoaderLoading() const;
    bool isInstalling() const;
    int progress() const;
    QString statusText() const;
    QVariantMap selectedInfo() const;
    int versionCount() const;

    Q_INVOKABLE void loadMockVersions();
    Q_INVOKABLE void refreshVersions();
    Q_INVOKABLE void chooseMinecraftDir();
    Q_INVOKABLE void chooseJavaPath();
    Q_INVOKABLE void startInstall();
    Q_INVOKABLE void cancelInstall();
    Q_INVOKABLE void openMinecraftDir();
    Q_INVOKABLE QString instanceId() const;
    static bool isRejectedGameDir(const QString& path);

signals:
    void latestChanged();
    void selectedVersionChanged();
    void minecraftDirChanged();
    void javaPathChanged();
    void loaderKindChanged();
    void loaderVersionChanged();
    void loaderLoadingChanged();
    void installingChanged();
    void progressChanged();
    void statusTextChanged();
    void error(const QString& message);

private:
    void setVersions(const QVector<VersionInfo>& versions);
    QVariantMap versionToMap(const VersionInfo& version) const;
    VersionInfo latestOfGroup(VersionGroup group) const;
    void setStatus(const QString& status);
    void tryFinishManifest();
    void refreshLoaderVersions();
    void setLoaderLoading(bool loading);
    void handleLoaderIndexFinished(const QByteArray& body);
    void handleLoaderIndexFailed(const QString& error);
    void handleInstallProgress(qint64 received, qint64 total, const QString& currentFile);
    void handleInstallFinished(const QString& versionId);
    void handleInstallFailed(const QString& message);
    void handleInstallCanceled();

    AppSettings* m_settings = nullptr;
    VersionListModel* m_versionModel = nullptr;
    LoaderListModel* m_loaderModel = nullptr;
    QVector<VersionInfo> m_versions;
    QString m_selectedVersion;
    QString m_minecraftDir;
    QString m_javaPath;
    QString m_loaderKind;
    QString m_loaderVersion;
    bool m_loaderLoading = false;
    bool m_installing = false;
    int m_progress = 0;
    QString m_statusText;

    QThread* m_installThread = nullptr;
    InstallController* m_installController = nullptr;

    JsonFetcher* m_bmclapiFetcher = nullptr;
    JsonFetcher* m_mojangFetcher = nullptr;
    JsonFetcher* m_loaderFetcher = nullptr;
    QByteArray m_bmclapiBody;
    QByteArray m_mojangBody;
    bool m_bmclapiDone = false;
    bool m_mojangDone = false;
};

#endif // APPCONTROLLER_H
