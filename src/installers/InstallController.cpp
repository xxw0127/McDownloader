#include "installers/InstallController.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include "core/Paths.h"
#include "core/VersionManifest.h"
#include "download/DownloadService.h"
#include "download/JsonFetcher.h"
#include "download/MirrorResolver.h"
#include "installers/InstallPlan.h"
#include "installers/VanillaInstallValidator.h"
#include "installers/VanillaPlanBuilder.h"

InstallController::InstallController(QObject* parent)
    : QObject(parent)
    , m_jsonFetcher(new JsonFetcher(this))
    , m_downloadService(new DownloadService(this))
{
    connect(m_jsonFetcher, &JsonFetcher::finished, this, &InstallController::handleJsonFinished);
    connect(m_jsonFetcher, &JsonFetcher::failed, this, &InstallController::handleJsonFailed);
    connect(m_downloadService, &DownloadService::progress, this, &InstallController::handleDownloadProgress);
    connect(m_downloadService, &DownloadService::finished, this, &InstallController::handleDownloadFinished);
    connect(m_downloadService, &DownloadService::failed, this, &InstallController::handleDownloadFailed);
    connect(m_downloadService, &DownloadService::canceled, this, &InstallController::handleDownloadCanceled);
}

bool InstallController::isRunning() const
{
    return m_stage != Stage::Idle;
}

void InstallController::start(const InstallRequest& request)
{
    if (isRunning())
    {
        emit failed(QStringLiteral("安装已在进行"));
        return;
    }

    if (request.loader != LoaderKind::Vanilla)
    {
        emit failed(QStringLiteral("该加载器尚未实现：%1").arg(loaderKindName(request.loader)));
        return;
    }

    if (request.mcVersion.trimmed().isEmpty())
    {
        emit failed(QStringLiteral("未选择 Minecraft 版本"));
        return;
    }

    const QDir target(request.minecraftDir);
    m_minecraftDir = target.absolutePath();
    if (!QDir().mkpath(m_minecraftDir))
    {
        emit failed(QStringLiteral("无法创建游戏文件夹：%1").arg(m_minecraftDir));
        return;
    }

    if (!QFileInfo(m_minecraftDir).isWritable())
    {
        emit failed(QStringLiteral("游戏文件夹不可写：%1").arg(m_minecraftDir));
        return;
    }

    m_request = request;
    m_versionId = request.mcVersion;
    m_versionJson = QJsonObject();
    m_assetIndex = QJsonObject();
    m_plan = InstallPlan();
    m_canceled = false;
    m_stage = Stage::Manifest;

    m_jsonFetcher->fetch(MirrorResolver::manifestUrls(), 20000);
}

void InstallController::cancel()
{
    if (m_stage == Stage::Idle)
        return;

    m_canceled = true;
    if (m_stage == Stage::Downloading)
    {
        m_downloadService->cancel();
        return;
    }

    m_jsonFetcher->cancel();
    m_stage = Stage::Idle;
    emit canceled();
}

void InstallController::handleJsonFinished(const QByteArray& body)
{
    if (m_canceled)
        return;

    if (m_stage == Stage::Manifest)
    {
        QString error;
        const QVector<VersionInfo> versions = VersionManifest::parse(body, &error);
        if (!error.isEmpty())
        {
            fail(error);
            return;
        }

        VersionInfo selected;
        bool found = false;
        for (const VersionInfo& version : versions)
        {
            if (version.id == m_request.mcVersion)
            {
                selected = version;
                found = true;
                break;
            }
        }

        if (!found)
        {
            fail(QStringLiteral("版本清单中找不到 %1").arg(m_request.mcVersion));
            return;
        }

        if (selected.url.isEmpty())
        {
            fail(QStringLiteral("版本 %1 缺少 JSON 地址").arg(m_request.mcVersion));
            return;
        }

        m_versionUrl = selected.url;
        m_stage = Stage::VersionJson;
        m_jsonFetcher->fetch(MirrorResolver::metaUrls(m_versionUrl), 20000);
        return;
    }

    if (m_stage == Stage::VersionJson)
    {
        const QJsonDocument document = QJsonDocument::fromJson(body);
        if (!document.isObject())
        {
            fail(QStringLiteral("版本 JSON 无效：%1").arg(m_request.mcVersion));
            return;
        }

        m_versionJson = document.object();
        const QJsonObject assetIndex = m_versionJson.value(QStringLiteral("assetIndex")).toObject();
        if (!assetIndex.isEmpty() && !assetIndex.value(QStringLiteral("url")).toString().isEmpty())
        {
            m_stage = Stage::AssetIndex;
            m_jsonFetcher->fetch(MirrorResolver::metaUrls(QUrl(assetIndex.value(QStringLiteral("url")).toString())), 20000);
            return;
        }

        startDownload();
        return;
    }

    if (m_stage == Stage::AssetIndex)
    {
        const QJsonDocument document = QJsonDocument::fromJson(body);
        if (!document.isObject())
        {
            fail(QStringLiteral("资源索引 JSON 无效"));
            return;
        }

        m_assetIndex = document.object();
        startDownload();
    }
}

void InstallController::handleJsonFailed(const QString& error)
{
    if (m_canceled)
        return;
    fail(error);
}

void InstallController::handleDownloadProgress(qint64 received, qint64 total, const QString& currentFile)
{
    emit progress(received, total, currentFile);
}

void InstallController::handleDownloadFinished()
{
    if (m_canceled)
        return;

    QString error;
    if (!applyCopies(&error) || !writeLauncherProfiles(&error))
    {
        fail(error);
        return;
    }

    const QString validation = VanillaInstallValidator::validate(m_minecraftDir, m_versionId);
    if (!validation.isEmpty())
    {
        fail(validation);
        return;
    }

    m_stage = Stage::Idle;
    emit finished(m_versionId);
}

void InstallController::handleDownloadFailed(const QString& error)
{
    if (m_canceled)
        return;
    fail(error);
}

void InstallController::handleDownloadCanceled()
{
    m_stage = Stage::Idle;
    emit canceled();
}

void InstallController::startDownload()
{
    m_plan = VanillaPlanBuilder::build(m_versionJson, m_minecraftDir, m_versionId, m_assetIndex);

    QString error;
    if (!writeVersionJson(&error))
    {
        fail(error);
        return;
    }

    m_stage = Stage::Downloading;
    m_downloadService->start(m_minecraftDir, m_plan.files);
}

bool InstallController::writeVersionJson(QString* error)
{
    bool ok = false;
    const QString relative = QStringLiteral("versions/%1/%1.json").arg(m_versionId);
    const QString target = Paths::joinWithin(m_minecraftDir, relative, &ok);
    if (!ok)
    {
        *error = QStringLiteral("版本 JSON 路径不安全：%1").arg(relative);
        return false;
    }

    if (!QDir().mkpath(QFileInfo(target).absolutePath()))
    {
        *error = QStringLiteral("无法创建版本目录：%1").arg(QFileInfo(target).absolutePath());
        return false;
    }

    QSaveFile file(target);
    if (!file.open(QIODevice::WriteOnly))
    {
        *error = QStringLiteral("无法写入版本 JSON：%1").arg(target);
        return false;
    }

    file.write(QJsonDocument(m_versionJson).toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        *error = QStringLiteral("提交版本 JSON 失败：%1").arg(target);
        return false;
    }

    return true;
}

bool InstallController::writeLauncherProfiles(QString* error)
{
    bool ok = false;
    const QString target = Paths::joinWithin(m_minecraftDir, QStringLiteral("launcher_profiles.json"), &ok);
    if (!ok)
    {
        *error = QStringLiteral("launcher_profiles.json 路径不安全");
        return false;
    }

    if (QFileInfo::exists(target))
        return true;

    QJsonObject profiles;
    profiles.insert(QStringLiteral("profiles"), QJsonObject());
    profiles.insert(QStringLiteral("selectedProfile"), QString());
    profiles.insert(QStringLiteral("clientToken"), QString());
    profiles.insert(QStringLiteral("authenticationDatabase"), QJsonObject());
    QJsonObject launcherVersion;
    launcherVersion.insert(QStringLiteral("name"), QStringLiteral("PCL2Downloader"));
    launcherVersion.insert(QStringLiteral("format"), 21);
    profiles.insert(QStringLiteral("launcherVersion"), launcherVersion);

    QSaveFile file(target);
    if (!file.open(QIODevice::WriteOnly))
    {
        *error = QStringLiteral("无法写入 launcher_profiles.json");
        return false;
    }
    file.write(QJsonDocument(profiles).toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        *error = QStringLiteral("提交 launcher_profiles.json 失败");
        return false;
    }
    return true;
}

bool InstallController::applyCopies(QString* error)
{
    for (const CopyOperation& copy : m_plan.copies)
    {
        bool sourceOk = false;
        bool targetOk = false;
        const QString source = Paths::joinWithin(m_minecraftDir, copy.sourceRelativePath, &sourceOk);
        const QString target = Paths::joinWithin(m_minecraftDir, copy.targetRelativePath, &targetOk);
        if (!sourceOk || !targetOk)
        {
            *error = QStringLiteral("资源复制路径不安全：%1").arg(copy.targetRelativePath);
            return false;
        }

        if (!QFileInfo::exists(source))
        {
            *error = QStringLiteral("资源复制源不存在：%1").arg(source);
            return false;
        }

        if (!QDir().mkpath(QFileInfo(target).absolutePath()))
        {
            *error = QStringLiteral("无法创建资源目录：%1").arg(QFileInfo(target).absolutePath());
            return false;
        }

        QFile::remove(target);
        if (!QFile::copy(source, target))
        {
            *error = QStringLiteral("复制资源失败：%1").arg(target);
            return false;
        }
    }
    return true;
}

void InstallController::fail(const QString& error)
{
    m_stage = Stage::Idle;
    emit failed(error);
}
