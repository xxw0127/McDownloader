#ifndef INSTALLCONTROLLER_H
#define INSTALLCONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>

#include "core/LoaderKind.h"
#include "installers/InstallPlan.h"

class DownloadService;
class JsonFetcher;

struct InstallRequest
{
    QString minecraftDir;
    QString mcVersion;
    LoaderKind loader = LoaderKind::Vanilla;
    QString loaderVersion;
    QString javaPath;
};

class InstallController : public QObject
{
    Q_OBJECT

public:
    explicit InstallController(QObject* parent = nullptr);

    void start(const InstallRequest& request);
    void cancel();
    bool isRunning() const;

signals:
    void progress(qint64 received, qint64 total, const QString& status);
    void finished(const QString& versionId);
    void failed(const QString& error);
    void canceled();

private:
    enum class Stage
    {
        Idle,
        Manifest,
        VersionJson,
        AssetIndex,
        DownloadingVanilla,
        LoaderIndex,
        LoaderProfile,
        DownloadingLoader,
        ForgeInstalling
    };

    enum class DownloadPhase
    {
        Vanilla,
        Loader
    };

    void handleJsonFinished(const QByteArray& body);
    void handleJsonFailed(const QString& error);
    void handleDownloadProgress(qint64 received, qint64 total, const QString& currentFile);
    void handleDownloadFinished();
    void handleDownloadFailed(const QString& error);
    void handleDownloadCanceled();
    void startDownload();
    void startLoaderPhase();
    void startForgeInstall();
    void startNeoForgeInstall();
    bool writeVersionJson(QString* error);
    bool writeJsonObject(const QString& versionId, const QJsonObject& object, QString* error);
    bool writeLauncherProfiles(QString* error);
    bool applyCopies(QString* error);
    void finishInstall();
    void fail(const QString& error);

    JsonFetcher* m_jsonFetcher = nullptr;
    DownloadService* m_downloadService = nullptr;
    InstallRequest m_request;
    InstallPlan m_plan;
    QJsonObject m_versionJson;
    QJsonObject m_assetIndex;
    QJsonObject m_loaderProfile;
    QUrl m_versionUrl;
    QString m_minecraftDir;
    QString m_versionId;
    QString m_loaderId;
    Stage m_stage = Stage::Idle;
    DownloadPhase m_downloadPhase = DownloadPhase::Vanilla;
    bool m_canceled = false;
};

#endif // INSTALLCONTROLLER_H
