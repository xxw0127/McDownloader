#ifndef INSTALLCONTROLLER_H
#define INSTALLCONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>

#include <atomic>

#include "installers/InstallPlan.h"

class DownloadService;
class JsonFetcher;

struct InstallRequest
{
    QString minecraftDir;
    QString mcVersion;
    QString javaPath;
};

Q_DECLARE_METATYPE(InstallRequest)

class InstallController : public QObject
{
    Q_OBJECT

public:
    explicit InstallController(QObject* parent = nullptr);

    Q_INVOKABLE void start(const InstallRequest& request);
    Q_INVOKABLE void cancel();
    void requestCancel();
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
        DownloadingVanilla
    };

    void handleJsonFinished(const QByteArray& body);
    void handleJsonFailed(const QString& error);
    void handleDownloadProgress(qint64 received, qint64 total, const QString& currentFile);
    void handleDownloadFinished();
    void handleDownloadFailed(const QString& error);
    void handleDownloadCanceled();
    void startDownload();
    bool writeVersionJson(QString* error);
    bool writeJsonObject(const QString& versionId, const QJsonObject& object, QString* error);
    bool writeLauncherProfiles(QString* error);
    bool applyCopies(QString* error);
    bool protectVersion(const QString& versionId, QString* error);
    void restoreProtectedVersion();
    void releaseProtectedVersion();
    void configureJava(const QString& versionId);
    bool createJavaLink(const QString& versionDir, const QString& javaPath, QString* error);
    bool writePclJavaConfig(const QString& versionDir, const QString& javaPath, QString* error);
    void finishInstall();
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
    bool m_manifestRetry = false;
    std::atomic_bool m_cancelRequested{false};
    QString m_protectedVersionId;
    QString m_backupPath;
    QString m_createdVersionId;
};

#endif // INSTALLCONTROLLER_H
