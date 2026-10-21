#include "installers/InstallController.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>

#include "core/Logger.h"
#include "core/Paths.h"
#include "core/VersionManifest.h"
#include "download/DownloadService.h"
#include "download/JsonFetcher.h"
#include "download/MirrorResolver.h"
#include "installers/InstallPlan.h"
#include "installers/ForgeInstaller.h"
#include "installers/JavaLocator.h"
#include "installers/JavaRequirement.h"
#include "installers/LoaderCatalog.h"
#include "installers/NeoForgeInstaller.h"
#include "installers/ProcessRunner.h"
#include "installers/ProfileInstaller.h"
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

    if (request.loader != LoaderKind::Vanilla &&
        request.loader != LoaderKind::Fabric &&
        request.loader != LoaderKind::Quilt &&
        request.loader != LoaderKind::Forge &&
        request.loader != LoaderKind::NeoForge)
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
    m_loaderProfile = QJsonObject();
    m_loaderId.clear();
    m_protectedVersionId.clear();
    m_backupPath.clear();
    m_createdVersionId.clear();
    m_plan = InstallPlan();
    m_canceled = false;
    m_manifestRetry = false;
    m_cancelRequested.store(false);
    m_downloadPhase = DownloadPhase::Vanilla;

    // 加载器安装优先复用已完整的父版本，避免覆盖父版本或重复下载。
    if (request.loader != LoaderKind::Vanilla)
    {
        const QString baseError = VanillaInstallValidator::validate(m_minecraftDir, m_versionId);
        if (baseError.isEmpty())
        {
            const QString versionJsonPath = QDir(m_minecraftDir)
                                                .filePath(QStringLiteral("versions/%1/%1.json").arg(m_versionId));
            QFile versionFile(versionJsonPath);
            if (versionFile.open(QIODevice::ReadOnly))
            {
                const QJsonDocument document = QJsonDocument::fromJson(versionFile.readAll());
                if (document.isObject())
                {
                    m_versionJson = document.object();
                    m_stage = Stage::LoaderIndex;
                    startLoaderPhase();
                    return;
                }
            }
        }
    }

    m_stage = Stage::Manifest;
    m_jsonFetcher->fetch(MirrorResolver::manifestUrls(), 20000);
}

void InstallController::requestCancel()
{
    // 可从 UI 线程直接调用；只写原子标志，不触碰工作线程对象。
    m_cancelRequested.store(true);
}

void InstallController::cancel()
{
    requestCancel();
    if (m_stage == Stage::Idle)
        return;

    m_canceled = true;
    if (m_stage == Stage::DownloadingVanilla || m_stage == Stage::DownloadingLoader)
    {
        m_downloadService->cancel();
        return;
    }
    if (m_stage == Stage::ForgeInstalling)
        return;

    m_jsonFetcher->cancel();
    restoreProtectedVersion();
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
            if (!m_manifestRetry)
            {
                m_manifestRetry = true;
                const QList<QUrl> urls = MirrorResolver::manifestUrls();
                if (urls.size() >= 2)
                {
                    m_stage = Stage::Manifest;
                    m_jsonFetcher->fetch({urls.at(1)}, 20000);
                    return;
                }
            }
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
        return;
    }

    if (m_stage == Stage::LoaderIndex)
    {
        QString error;
        const QVector<LoaderVersion> versions = LoaderCatalog::parseIndex(m_request.loader,
                                                                         body,
                                                                         m_versionId,
                                                                         &error);
        if (!error.isEmpty() || versions.isEmpty())
        {
            fail(error.isEmpty() ? QStringLiteral("没有可用的加载器版本") : error);
            return;
        }

        m_request.loaderVersion = versions.first().version;
        if (m_request.loader == LoaderKind::Forge)
        {
            startForgeInstall();
            return;
        }
        if (m_request.loader == LoaderKind::NeoForge)
        {
            startNeoForgeInstall();
            return;
        }

        const QList<QUrl> urls = LoaderCatalog::profileUrls(m_request.loader,
                                                            m_versionId,
                                                            m_request.loaderVersion);
        if (urls.isEmpty())
        {
            fail(QStringLiteral("加载器 profile 地址为空"));
            return;
        }

        m_stage = Stage::LoaderProfile;
        m_jsonFetcher->fetch(urls, 20000);
        return;
    }

    if (m_stage == Stage::LoaderProfile)
    {
        m_loaderId = ProfileInstaller::instanceId(m_request.loader,
                                                  m_versionId,
                                                  m_request.loaderVersion);
        if (m_loaderId.isEmpty())
        {
            fail(QStringLiteral("无法生成加载器实例名"));
            return;
        }

        QString error;
        const QJsonObject profile = ProfileInstaller::prepareProfile(body, m_loaderId, &error);
        if (!error.isEmpty())
        {
            fail(error);
            return;
        }

        if (!protectVersion(m_loaderId, &error))
        {
            fail(error);
            return;
        }

        if (!writeJsonObject(m_loaderId, profile, &error))
        {
            fail(error);
            return;
        }

        m_loaderProfile = profile;
        m_plan = ProfileInstaller::buildPlan(profile, m_minecraftDir, m_loaderId);
        m_downloadPhase = DownloadPhase::Loader;
        m_stage = Stage::DownloadingLoader;
        m_downloadService->start(m_minecraftDir, m_plan.files);
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
    if (!writeLauncherProfiles(&error))
    {
        fail(error);
        return;
    }

    if (m_downloadPhase == DownloadPhase::Vanilla)
    {
        if (!applyCopies(&error))
        {
            fail(error);
            return;
        }

        if (m_request.loader != LoaderKind::Vanilla)
        {
            startLoaderPhase();
            return;
        }
    }

    const QString validation = VanillaInstallValidator::validate(m_minecraftDir, m_versionId);
    if (!validation.isEmpty())
    {
        fail(validation);
        return;
    }

    if (m_request.loader != LoaderKind::Vanilla && !m_loaderId.isEmpty())
    {
        bool ok = false;
        const QString loaderPath = Paths::joinWithin(m_minecraftDir,
                                                     QStringLiteral("versions/%1/%1.json").arg(m_loaderId),
                                                     &ok);
        if (!ok || !QFileInfo::exists(loaderPath))
        {
            fail(QStringLiteral("加载器版本 JSON 不存在：%1").arg(m_loaderId));
            return;
        }

        QFile loaderFile(loaderPath);
        if (!loaderFile.open(QIODevice::ReadOnly) || !QJsonDocument::fromJson(loaderFile.readAll()).isObject())
        {
            fail(QStringLiteral("加载器版本 JSON 无效：%1").arg(m_loaderId));
            return;
        }
    }

    finishInstall();
}

void InstallController::handleDownloadFailed(const QString& error)
{
    if (m_canceled)
        return;
    fail(error);
}

void InstallController::handleDownloadCanceled()
{
    restoreProtectedVersion();
    m_stage = Stage::Idle;
    emit canceled();
}

void InstallController::startDownload()
{
    m_plan = VanillaPlanBuilder::build(m_versionJson, m_minecraftDir, m_versionId, m_assetIndex);

    QString error;
    if (!protectVersion(m_versionId, &error))
    {
        fail(error);
        return;
    }

    if (!writeVersionJson(&error))
    {
        fail(error);
        return;
    }

    m_downloadPhase = DownloadPhase::Vanilla;
    m_stage = Stage::DownloadingVanilla;
    m_downloadService->start(m_minecraftDir, m_plan.files);
}

void InstallController::startLoaderPhase()
{
    if (m_request.loaderVersion.isEmpty())
    {
        const QList<QUrl> urls = LoaderCatalog::indexUrls(m_request.loader, m_versionId);
        if (urls.isEmpty())
        {
            fail(QStringLiteral("没有可用的加载器列表地址"));
            return;
        }

        m_stage = Stage::LoaderIndex;
        m_jsonFetcher->fetch(urls, 20000);
        return;
    }

    if (m_request.loader == LoaderKind::Forge)
    {
        startForgeInstall();
        return;
    }
    if (m_request.loader == LoaderKind::NeoForge)
    {
        startNeoForgeInstall();
        return;
    }

    const QList<QUrl> urls = LoaderCatalog::profileUrls(m_request.loader,
                                                        m_versionId,
                                                        m_request.loaderVersion);
    if (urls.isEmpty())
    {
        fail(QStringLiteral("没有可用的加载器 profile 地址"));
        return;
    }

    m_stage = Stage::LoaderProfile;
    m_jsonFetcher->fetch(urls, 20000);
}

void InstallController::startForgeInstall()
{
    const int requiredJava = JavaRequirement::majorForVersion(m_versionJson, m_versionId);
    QString javaError;
    if (!validateJavaPath(m_request.javaPath, requiredJava, &javaError))
    {
        fail(javaError);
        return;
    }

    m_stage = Stage::ForgeInstalling;
    emit progress(0, 0, QStringLiteral("正在运行 Forge 安装器..."));

    const ForgeInstallResult result = ForgeInstaller::isLegacy(m_versionId)
                                          ? ForgeInstaller::installLegacy(m_minecraftDir,
                                                                          m_versionId,
                                                                          m_request.loaderVersion,
                                                                          &m_cancelRequested)
                                          : ForgeInstaller::installModern(m_minecraftDir,
                                                                          m_versionId,
                                                                          m_request.loaderVersion,
                                                                          m_request.javaPath,
                                                                          m_versionJson,
                                                                          &m_cancelRequested);
    if (m_canceled || m_cancelRequested.load())
    {
        m_stage = Stage::Idle;
        emit canceled();
        return;
    }

    if (!result.ok)
    {
        fail(result.error);
        return;
    }

    m_loaderId = result.versionId;
    finishInstall();
}

void InstallController::startNeoForgeInstall()
{
    if (!NeoForgeInstaller::isSupported(m_versionId))
    {
        fail(QStringLiteral("NeoForge 不支持 Minecraft %1").arg(m_versionId));
        return;
    }

    const int requiredJava = JavaRequirement::majorForVersion(m_versionJson, m_versionId);
    QString javaError;
    if (!validateJavaPath(m_request.javaPath, requiredJava, &javaError))
    {
        fail(javaError);
        return;
    }

    m_stage = Stage::ForgeInstalling;
    emit progress(0, 0, QStringLiteral("正在运行 NeoForge 安装器..."));

    const NeoForgeInstallResult result = NeoForgeInstaller::install(m_minecraftDir,
                                                                    m_versionId,
                                                                    m_request.loaderVersion,
                                                                    m_request.javaPath,
                                                                    m_versionJson,
                                                                    &m_cancelRequested);
    if (m_canceled || m_cancelRequested.load())
    {
        m_stage = Stage::Idle;
        emit canceled();
        return;
    }

    if (!result.ok)
    {
        fail(result.error);
        return;
    }

    m_loaderId = result.versionId;
    finishInstall();
}

bool InstallController::writeVersionJson(QString* error)
{
    return writeJsonObject(m_versionId, m_versionJson, error);
}

bool InstallController::writeJsonObject(const QString& versionId, const QJsonObject& object, QString* error)
{
    bool ok = false;
    const QString relative = QStringLiteral("versions/%1/%1.json").arg(versionId);
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

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        *error = QStringLiteral("提交版本 JSON 失败：%1").arg(target);
        return false;
    }

    return true;
}

bool InstallController::protectVersion(const QString& versionId, QString* error)
{
    bool ok = false;
    const QString target = Paths::joinWithin(m_minecraftDir,
                                             QStringLiteral("versions/%1").arg(versionId),
                                             &ok);
    if (!ok)
    {
        *error = QStringLiteral("版本路径不安全：%1").arg(versionId);
        return false;
    }

    if (!QFileInfo::exists(target))
    {
        m_createdVersionId = versionId;
        return true;
    }

    const QString backupRoot = Paths::joinWithin(m_minecraftDir,
                                                 QStringLiteral(".pcl2downloader-backup"),
                                                 &ok);
    if (!ok || !QDir().mkpath(backupRoot))
    {
        *error = QStringLiteral("无法创建版本备份目录");
        return false;
    }

    const QString backup = QDir(backupRoot).filePath(
        QStringLiteral("%1-%2").arg(versionId,
                                    QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"))));
    if (!QDir().rename(target, backup))
    {
        *error = QStringLiteral("无法备份已有版本：%1").arg(versionId);
        return false;
    }

    m_protectedVersionId = versionId;
    m_backupPath = backup;
    Logger::info(QStringLiteral("install"),
                 QStringLiteral("已备份已有版本 %1 到 %2").arg(versionId, backup));
    return true;
}

void InstallController::restoreProtectedVersion()
{
    if (!m_backupPath.isEmpty())
    {
        bool ok = false;
        const QString target = Paths::joinWithin(m_minecraftDir,
                                                 QStringLiteral("versions/%1").arg(m_protectedVersionId),
                                                 &ok);
        if (ok && !m_protectedVersionId.isEmpty())
        {
            QDir(target).removeRecursively();
            if (!QDir().rename(m_backupPath, target))
            {
                Logger::error(QStringLiteral("install"),
                              QStringLiteral("恢复旧版本失败，备份保留在 %1").arg(m_backupPath));
                return;
            }
            Logger::info(QStringLiteral("install"),
                         QStringLiteral("安装失败，已恢复旧版本 %1").arg(m_protectedVersionId));
        }
    }
    else if (!m_createdVersionId.isEmpty())
    {
        bool ok = false;
        const QString target = Paths::joinWithin(m_minecraftDir,
                                                 QStringLiteral("versions/%1").arg(m_createdVersionId),
                                                 &ok);
        if (ok)
            QDir(target).removeRecursively();
    }

    m_protectedVersionId.clear();
    m_backupPath.clear();
    m_createdVersionId.clear();
}

void InstallController::releaseProtectedVersion()
{
    if (!m_backupPath.isEmpty())
        Logger::info(QStringLiteral("install"),
                     QStringLiteral("旧版本备份保留在 %1").arg(m_backupPath));
    m_protectedVersionId.clear();
    m_backupPath.clear();
    m_createdVersionId.clear();
}

bool InstallController::validateJavaPath(const QString& javaPath,
                                         int requiredMajor,
                                         QString* error) const
{
    if (javaPath.isEmpty())
        return true;

    const QFileInfo info(javaPath);
    if (!info.exists() || !info.isFile())
    {
        *error = QStringLiteral("Java 可执行文件不存在：%1").arg(javaPath);
        return false;
    }

    const ProcessResult version = ProcessRunner::run(info.absoluteFilePath(),
                                                     {QStringLiteral("-version")},
                                                     QProcessEnvironment(),
                                                     5000);
    const JavaInfo java = JavaLocator::parseVersionOutput(version.stdOut + version.stdErr);
    if (java.major != requiredMajor || !java.is64Bit)
    {
        *error = QStringLiteral("需要 64 位 Java %1，当前选择不匹配：%2")
                     .arg(requiredMajor)
                     .arg(javaPath);
        return false;
    }
    return true;
}

void InstallController::configureJava(const QString& versionId)
{
    const int requiredMajor = JavaRequirement::majorForVersion(m_versionJson, m_versionId);

    JavaInfo selected;
    const QFileInfo requestedJava(m_request.javaPath);
    if (!m_request.javaPath.isEmpty() && requestedJava.exists() && requestedJava.isFile())
    {
        const ProcessResult version = ProcessRunner::run(requestedJava.absoluteFilePath(),
                                                         {QStringLiteral("-version")},
                                                         QProcessEnvironment(),
                                                         5000);
        JavaInfo requested = JavaLocator::parseVersionOutput(version.stdOut + version.stdErr);
        if (requested.major == requiredMajor && requested.is64Bit)
        {
            requested.path = requestedJava.absoluteFilePath();
            selected = requested;
        }
    }

    if (selected.path.isEmpty())
        selected = JavaLocator::selectForInstaller(JavaLocator::scan(m_minecraftDir), requiredMajor);

    if (selected.path.isEmpty())
    {
        Logger::warn(QStringLiteral("java"),
                     QStringLiteral("未找到 Java %1，版本 %2 需要在启动器中手动选择 Java")
                         .arg(requiredMajor)
                         .arg(versionId));
        return;
    }

    bool ok = false;
    const QString versionDir = Paths::joinWithin(m_minecraftDir,
                                                 QStringLiteral("versions/%1").arg(versionId),
                                                 &ok);
    if (!ok)
    {
        Logger::warn(QStringLiteral("java"), QStringLiteral("Java 配置路径不安全：%1").arg(versionId));
        return;
    }

    QString error;
    if (!createJavaLink(versionDir, selected.path, &error) ||
        !writePclJavaConfig(versionDir, selected.path, &error))
    {
        Logger::warn(QStringLiteral("java"),
                     QStringLiteral("版本 %1 的 Java 配置失败：%2").arg(versionId, error));
        return;
    }

    Logger::info(QStringLiteral("java"),
                 QStringLiteral("版本 %1 已绑定 Java %2：%3")
                     .arg(versionId)
                     .arg(requiredMajor)
                     .arg(selected.path));
}

bool InstallController::createJavaLink(const QString& versionDir,
                                       const QString& javaPath,
                                       QString* error)
{
    const QFileInfo javaInfo(javaPath);
    if (!javaInfo.exists() || !javaInfo.isFile())
    {
        *error = QStringLiteral("Java 可执行文件不存在：%1").arg(javaPath);
        return false;
    }

    QDir binDir = javaInfo.dir();
    if (!binDir.cdUp())
    {
        *error = QStringLiteral("无法定位 Java 根目录：%1").arg(javaPath);
        return false;
    }
    const QString javaRoot = binDir.absolutePath();
    const QString link = QDir(versionDir).filePath(QStringLiteral("java"));

    if (QFileInfo::exists(link) && QFileInfo::exists(QDir(link).filePath(QStringLiteral("bin/java.exe"))))
        return true;

#ifdef Q_OS_WIN
    if (QFileInfo::exists(link))
        QProcess::execute(QStringLiteral("cmd.exe"),
                          {QStringLiteral("/c"), QStringLiteral("rmdir"),
                           QDir::toNativeSeparators(link)});
    const int code = QProcess::execute(QStringLiteral("cmd.exe"),
                                       {QStringLiteral("/c"), QStringLiteral("mklink"),
                                        QStringLiteral("/J"), QDir::toNativeSeparators(link),
                                        QDir::toNativeSeparators(javaRoot)});
    if (code != 0)
    {
        *error = QStringLiteral("无法创建版本 Java 目录链接");
        return false;
    }
#else
    if (QFileInfo::exists(link))
        QDir(link).removeRecursively();
    if (!QFile::link(javaRoot, link))
    {
        *error = QStringLiteral("无法创建版本 Java 符号链接");
        return false;
    }
#endif
    return true;
}

bool InstallController::writePclJavaConfig(const QString& versionDir,
                                           const QString& javaPath,
                                           QString* error)
{
    const QString pclDir = QDir(versionDir).filePath(QStringLiteral("PCL"));
    if (!QDir().mkpath(pclDir))
    {
        *error = QStringLiteral("无法创建版本 PCL 配置目录");
        return false;
    }

    const QString iniPath = QDir(pclDir).filePath(QStringLiteral("Setup.ini"));
    QStringList lines;
    QFile existing(iniPath);
    if (existing.open(QIODevice::ReadOnly))
    {
        const QStringList oldLines = QString::fromUtf8(existing.readAll()).split(QLatin1Char('\n'));
        for (QString line : oldLines)
        {
            line.remove(QLatin1Char('\r'));
            if (line.startsWith(QLatin1String("VersionArgumentJavaV2:")) ||
                line.startsWith(QLatin1String("VersionArgumentJavaSelect:")))
                continue;
            if (!line.isEmpty())
                lines.append(line);
        }
    }

    QString javaBin = QFileInfo(javaPath).absolutePath();
    javaBin = QDir::fromNativeSeparators(javaBin);
    if (!javaBin.endsWith(QLatin1Char('/')))
        javaBin += QLatin1Char('/');

    lines.append(QStringLiteral("VersionArgumentJavaV2:2"));
    lines.append(QStringLiteral("VersionArgumentJavaSelect:{\"Path\":\"%1\"}").arg(javaBin));

    QFile file(iniPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        *error = QStringLiteral("无法写入版本 PCL 配置");
        return false;
    }
    file.write((lines.join(QStringLiteral("\r\n")) + QStringLiteral("\r\n")).toUtf8());
    file.flush();
    if (file.error() != QFile::NoError)
    {
        *error = QStringLiteral("写入版本 PCL 配置失败");
        return false;
    }
    return true;
}

void InstallController::finishInstall()
{
    const QString finalId = m_loaderId.isEmpty() ? m_versionId : m_loaderId;
    configureJava(finalId);
    releaseProtectedVersion();
    m_stage = Stage::Idle;
    emit finished(finalId);
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
    const QString backup = m_backupPath;
    restoreProtectedVersion();
    QString message = error;
    if (!backup.isEmpty() && QFileInfo::exists(backup))
        message += QStringLiteral("（旧版本已备份到 %1）").arg(backup);
    m_stage = Stage::Idle;
    emit failed(message);
}
