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
        Downloading
    };

    void handleJsonFinished(const QByteArray& body);
    void handleJsonFailed(const QString& error);
    void handleDownloadProgress(qint64 received, qint64 total, const QString& currentFile);
    void handleDownloadFinished();
    void handleDownloadFailed(const QString& error);
    void handleDownloadCanceled();
    void startDownload();
    bool writeVersionJson(QString* error);
    bool writeLauncherProfiles(QString* error);
    bool applyCopies(QString* error);
    void fail(const QString& error);

    JsonFetcher* m_jsonFetcher = nullptr;
    DownloadService* m_downloadService = nullptr;
    InstallRequest m_request;
    InstallPlan m_plan;
    QJsonObject m_versionJson;
    QJsonObject m_assetIndex;
    QUrl m_versionUrl;
    QString m_minecraftDir;
    QString m_versionId;
    Stage m_stage = Stage::Idle;
    bool m_canceled = false;
};

#endif // INSTALLCONTROLLER_H
