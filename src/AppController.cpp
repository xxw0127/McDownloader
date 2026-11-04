#include "AppController.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMetaObject>
#include <QThread>
#include <QUrl>

#include "core/AppSettings.h"
#include "core/Logger.h"
#include "core/VersionGrouping.h"
#include "core/VersionManifest.h"
#include "download/JsonFetcher.h"
#include "download/MirrorResolver.h"
#include "installers/LoaderCatalog.h"
#include "installers/NeoForgeInstaller.h"
#include "models/LoaderListModel.h"
#include "models/VersionListModel.h"

AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_settings(new AppSettings(this))
    , m_versionModel(new VersionListModel(this))
    , m_loaderModel(new LoaderListModel(this))
    , m_loaderKind(QStringLiteral("vanilla"))
{
    qRegisterMetaType<InstallRequest>("InstallRequest");

    m_minecraftDir = m_settings->minecraftDir();
    if (m_minecraftDir.isEmpty() || isRejectedGameDir(m_minecraftDir))
        m_minecraftDir = QDir::homePath() + QStringLiteral("/.minecraft");
    QDir().mkpath(m_minecraftDir);
    m_javaPath = m_settings->javaPath();

    m_installThread = new QThread(this);
    m_installController = new InstallController();
    m_installController->moveToThread(m_installThread);
    connect(m_installController, &InstallController::progress, this, &AppController::handleInstallProgress);
    connect(m_installController, &InstallController::finished, this, &AppController::handleInstallFinished);
    connect(m_installController, &InstallController::failed, this, &AppController::handleInstallFailed);
    connect(m_installController, &InstallController::canceled, this, &AppController::handleInstallCanceled);
    connect(m_installThread, &QThread::finished, m_installController, &QObject::deleteLater);
    m_installThread->start();

    m_bmclapiFetcher = new JsonFetcher(this);
    m_mojangFetcher = new JsonFetcher(this);
    connect(m_bmclapiFetcher, &JsonFetcher::finished, this, [this](const QByteArray& body) {
        m_bmclapiBody = body;
        m_bmclapiDone = true;
        tryFinishManifest();
    });
    connect(m_bmclapiFetcher, &JsonFetcher::failed, this, [this](const QString&) {
        m_bmclapiBody.clear();
        m_bmclapiDone = true;
        tryFinishManifest();
    });
    connect(m_mojangFetcher, &JsonFetcher::finished, this, [this](const QByteArray& body) {
        m_mojangBody = body;
        m_mojangDone = true;
        tryFinishManifest();
    });
    connect(m_mojangFetcher, &JsonFetcher::failed, this, [this](const QString&) {
        m_mojangBody.clear();
        m_mojangDone = true;
        tryFinishManifest();
    });

    m_loaderFetcher = new JsonFetcher(this);
    connect(m_loaderFetcher, &JsonFetcher::finished, this, &AppController::handleLoaderIndexFinished);
    connect(m_loaderFetcher, &JsonFetcher::failed, this, &AppController::handleLoaderIndexFailed);
}

AppController::~AppController()
{
    if (m_installController != nullptr)
    {
        m_installController->requestCancel();
        QMetaObject::invokeMethod(m_installController, "cancel", Qt::QueuedConnection);
    }

    if (m_installThread != nullptr)
    {
        m_installThread->quit();
        if (!m_installThread->wait(30000))
        {
            // 避免析构仍在运行的 QThread 触发 qFatal；进程退出时由系统回收。
            m_installThread->setParent(nullptr);
        }
    }
}

bool AppController::isRejectedGameDir(const QString& path)
{
    if (path.trimmed().isEmpty())
        return false;

    const QString normalized = QDir::cleanPath(QFileInfo(path).absoluteFilePath()).toLower();
    const QString appDir = QDir::cleanPath(QCoreApplication::applicationDirPath()).toLower();
    const QString systemRoot = QDir::cleanPath(qEnvironmentVariable("SystemRoot")).toLower();
    const bool isProgramDir = normalized == appDir || normalized.startsWith(appDir + QLatin1Char('/'));
    const bool isWindowsDir = !systemRoot.isEmpty() &&
                              (normalized == systemRoot ||
                               normalized.startsWith(systemRoot + QLatin1Char('/')));
    return isProgramDir || isWindowsDir;
}

QString AppController::appName() const
{
    return QStringLiteral("McDownloader");
}

QAbstractItemModel* AppController::versionModel()
{
    return m_versionModel;
}

QAbstractItemModel* AppController::loaderModel()
{
    return m_loaderModel;
}

QVariantMap AppController::latestRelease() const
{
    return versionToMap(latestOfGroup(VersionGroup::Release));
}

QVariantMap AppController::latestSnapshot() const
{
    const VersionInfo release = latestOfGroup(VersionGroup::Release);
    const VersionInfo snapshot = latestOfGroup(VersionGroup::Snapshot);
    if (snapshot.id.isEmpty())
        return {};
    if (!release.id.isEmpty() && snapshot.releaseTime <= release.releaseTime)
        return {};
    return versionToMap(snapshot);
}

QString AppController::selectedVersion() const
{
    return m_selectedVersion;
}

void AppController::setSelectedVersion(const QString& versionId)
{
    if (m_selectedVersion == versionId)
        return;

    m_selectedVersion = versionId;
    m_versionModel->setSelectedId(versionId);
    emit selectedVersionChanged();
    emit latestChanged();

    if (loaderKindFromName(m_loaderKind) != LoaderKind::Vanilla)
        refreshLoaderVersions();
}

QString AppController::minecraftDir() const
{
    return m_minecraftDir;
}

void AppController::setMinecraftDir(const QString& path)
{
    if (path.trimmed().isEmpty())
        return;

    const QFileInfo info(path);
    if (!info.exists() || !info.isDir())
    {
        setStatus(QStringLiteral("文件夹不存在或不是目录：%1").arg(path));
        emit error(QStringLiteral("请选择存在的 .minecraft 文件夹"));
        return;
    }

    const QString absolute = info.absoluteFilePath();
    if (m_minecraftDir == absolute)
        return;

    if (isRejectedGameDir(absolute))
    {
        setStatus(QStringLiteral("不能选择程序目录或 Windows 系统目录：%1").arg(path));
        emit error(QStringLiteral("请选择其他 .minecraft 文件夹"));
        return;
    }

    m_minecraftDir = absolute;
    m_settings->setMinecraftDir(absolute);
    emit minecraftDirChanged();
}

QString AppController::javaPath() const
{
    return m_javaPath;
}

void AppController::setJavaPath(const QString& path)
{
    if (m_javaPath == path)
        return;
    m_javaPath = path;
    m_settings->setJavaPath(path);
    emit javaPathChanged();
}

QString AppController::loaderKind() const
{
    return m_loaderKind;
}

void AppController::setLoaderKind(const QString& kind)
{
    const QString normalized = kind.trimmed().toLower();
    if (m_loaderKind == normalized)
        return;

    m_loaderKind = normalized;
    m_loaderVersion.clear();
    m_loaderModel->setVersions({});
    emit loaderKindChanged();
    emit loaderVersionChanged();

    if (loaderKindFromName(m_loaderKind) == LoaderKind::Vanilla)
    {
        setLoaderLoading(false);
        setStatus(QStringLiteral("已选择原版"));
        return;
    }

    refreshLoaderVersions();
}

QString AppController::loaderVersion() const
{
    return m_loaderVersion;
}

void AppController::setLoaderVersion(const QString& version)
{
    if (m_loaderVersion == version)
        return;
    m_loaderVersion = version;
    emit loaderVersionChanged();
}

bool AppController::isInstalling() const
{
    return m_installing;
}

int AppController::progress() const
{
    return m_progress;
}

QString AppController::statusText() const
{
    return m_statusText;
}

void AppController::loadMockVersions()
{
    QFile file(QStringLiteral(":/fixtures/manifest_small.json"));
    if (!file.open(QIODevice::ReadOnly))
    {
        setStatus(QStringLiteral("无法打开内置版本 fixture"));
        return;
    }

    QString error;
    const QVector<VersionInfo> versions = VersionManifest::parse(file.readAll(), &error);
    if (!error.isEmpty())
    {
        setStatus(error);
        return;
    }

    setVersions(versions);
    setStatus(QStringLiteral("已加载 %1 个演示版本").arg(versions.size()));
}

void AppController::refreshVersions()
{
    m_bmclapiBody.clear();
    m_mojangBody.clear();
    m_bmclapiDone = false;
    m_mojangDone = false;
    setStatus(QStringLiteral("正在获取版本列表..."));

    const QList<QUrl> urls = MirrorResolver::manifestUrls();
    if (urls.size() < 2)
    {
        setStatus(QStringLiteral("版本源配置无效"));
        return;
    }

    m_bmclapiFetcher->fetch({urls.at(0)}, 20000);
    m_mojangFetcher->fetch({urls.at(1)}, 20000);
}

void AppController::chooseMinecraftDir()
{
    const QString dir = QFileDialog::getExistingDirectory(nullptr,
                                                          QStringLiteral("选择 .minecraft 文件夹"),
                                                          m_minecraftDir);
    if (!dir.isEmpty())
        setMinecraftDir(dir);
}

void AppController::chooseJavaPath()
{
    const QString path = QFileDialog::getOpenFileName(nullptr,
                                                       QStringLiteral("选择 java.exe"),
                                                       m_javaPath,
                                                       QStringLiteral("Java (java.exe);;所有文件 (*)"));
    if (!path.isEmpty())
        setJavaPath(path);
}

void AppController::startInstall()
{
    if (m_installing)
        return;

    if (m_selectedVersion.isEmpty())
    {
        setStatus(QStringLiteral("请先选择一个 Minecraft 版本"));
        emit error(QStringLiteral("请先选择版本"));
        return;
    }

    if (m_minecraftDir.isEmpty())
    {
        setStatus(QStringLiteral("请先选择 .minecraft 文件夹"));
        emit error(QStringLiteral("请先选择文件夹"));
        return;
    }

    InstallRequest request;
    request.minecraftDir = m_minecraftDir;
    request.mcVersion = m_selectedVersion;
    request.loader = loaderKindFromName(m_loaderKind);
    request.loaderVersion = m_loaderVersion;
    request.javaPath = m_javaPath;

    m_installing = true;
    m_progress = 0;
    emit installingChanged();
    emit progressChanged();
    setStatus(QStringLiteral("准备安装 %1...").arg(m_selectedVersion));

    const bool invoked = QMetaObject::invokeMethod(m_installController, "start", Qt::QueuedConnection,
                                                   Q_ARG(InstallRequest, request));
    if (!invoked)
    {
        m_installing = false;
        emit installingChanged();
        setStatus(QStringLiteral("无法启动安装线程"));
        emit error(QStringLiteral("无法启动安装线程"));
    }
}

void AppController::cancelInstall()
{
    if (!m_installing)
        return;
    setStatus(QStringLiteral("正在取消..."));
    // 工作线程可能阻塞在安装器进程里，先直接置取消标志。
    m_installController->requestCancel();
    QMetaObject::invokeMethod(m_installController, "cancel", Qt::QueuedConnection);
}

void AppController::openMinecraftDir()
{
    if (!m_minecraftDir.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_minecraftDir));
}

QString AppController::instanceId() const
{
    if (m_selectedVersion.isEmpty())
        return QString();

    switch (loaderKindFromName(m_loaderKind))
    {
    case LoaderKind::Vanilla:
        return m_selectedVersion;
    case LoaderKind::Fabric:
        return QStringLiteral("fabric-loader-%1-%2").arg(m_loaderVersion, m_selectedVersion);
    case LoaderKind::Quilt:
        return QStringLiteral("quilt-loader-%1-%2").arg(m_loaderVersion, m_selectedVersion);
    case LoaderKind::Forge:
        return QStringLiteral("%1-forge-%2").arg(m_selectedVersion, m_loaderVersion);
    case LoaderKind::NeoForge:
        return NeoForgeInstaller::instanceId(m_selectedVersion, m_loaderVersion);
    }
    return m_selectedVersion;
}

QVariantMap AppController::selectedInfo() const
{
    for (const VersionInfo& version : m_versions)
    {
        if (version.id != m_selectedVersion || version.id.isEmpty())
            continue;
        QVariantMap map = versionToMap(version);
        map.insert(QStringLiteral("groupName"),
                   VersionGrouping::displayName(VersionGrouping::classify(version)));
        return map;
    }
    return {};
}

int AppController::versionCount() const
{
    return m_versions.size();
}

void AppController::setVersions(const QVector<VersionInfo>& versions)
{
    m_versions = versions;
    m_versionModel->setVersions(versions);

    // A list that opens with every channel collapsed reads as an empty screen on
    // a 900-version manifest. Open the release channel and pre-select its newest
    // entry so the install rail has something to describe on first launch.
    m_versionModel->setGroupExpanded(VersionGroup::Release, true);
    if (m_selectedVersion.isEmpty() && !versions.isEmpty())
    {
        const VersionInfo latest = latestOfGroup(VersionGroup::Release);
        if (!latest.id.isEmpty())
            setSelectedVersion(latest.id);
    }

    emit latestChanged();
}

QVariantMap AppController::versionToMap(const VersionInfo& version) const
{
    if (version.id.isEmpty())
        return {};

    const VersionGroup group = VersionGrouping::classify(version);
    QVariantMap map;
    map.insert(QStringLiteral("id"), version.id);
    map.insert(QStringLiteral("displayId"), version.displayId);
    map.insert(QStringLiteral("type"), version.type);
    map.insert(QStringLiteral("releaseTime"), VersionGrouping::formatReleaseTime(version.releaseTime));
    map.insert(QStringLiteral("icon"), VersionGrouping::iconPath(group));
    map.insert(QStringLiteral("lore"), version.lore);
    map.insert(QStringLiteral("selected"), version.id == m_selectedVersion);
    return map;
}

VersionInfo AppController::latestOfGroup(VersionGroup group) const
{
    VersionInfo result;
    bool found = false;
    for (const VersionInfo& version : m_versions)
    {
        if (VersionGrouping::classify(version) != group)
            continue;
        if (!found || version.releaseTime > result.releaseTime)
        {
            result = version;
            found = true;
        }
    }
    return result;
}

void AppController::setStatus(const QString& status)
{
    if (m_statusText == status)
        return;
    m_statusText = status;
    emit statusTextChanged();
}

void AppController::refreshLoaderVersions()
{
    const LoaderKind kind = loaderKindFromName(m_loaderKind);
    if (kind == LoaderKind::Vanilla)
    {
        m_loaderModel->setVersions({});
        setLoaderVersion(QString());
        setLoaderLoading(false);
        return;
    }

    if (m_selectedVersion.isEmpty())
    {
        setLoaderLoading(false);
        setStatus(QStringLiteral("请先选择 Minecraft 版本"));
        return;
    }

    const QList<QUrl> urls = LoaderCatalog::indexUrls(kind, m_selectedVersion);
    if (urls.isEmpty())
    {
        setLoaderLoading(false);
        setStatus(QStringLiteral("该加载器没有可用的版本列表"));
        return;
    }

    setLoaderLoading(true);
    setStatus(QStringLiteral("正在获取 %1 版本...").arg(loaderKindName(kind)));
    m_loaderFetcher->fetch(urls, 20000);
}

void AppController::handleLoaderIndexFinished(const QByteArray& body)
{
    const LoaderKind kind = loaderKindFromName(m_loaderKind);
    QString error;
    const QVector<LoaderVersion> versions = LoaderCatalog::parseIndex(kind, body, m_selectedVersion, &error);
    if (!error.isEmpty() || versions.isEmpty())
    {
        m_loaderModel->setVersions({});
        setLoaderVersion(QString());
        setLoaderLoading(false);
        setStatus(error.isEmpty() ? QStringLiteral("没有可用的加载器版本") : error);
        return;
    }

    m_loaderModel->setVersions(versions);
    setLoaderVersion(versions.first().version);
    setLoaderLoading(false);
    setStatus(QStringLiteral("已加载 %1 个 %2 版本").arg(versions.size()).arg(loaderKindName(kind)));
}

void AppController::handleLoaderIndexFailed(const QString& error)
{
    m_loaderModel->setVersions({});
    setLoaderVersion(QString());
    setLoaderLoading(false);
    setStatus(QStringLiteral("加载器列表获取失败：%1").arg(error));
}

void AppController::setLoaderLoading(bool loading)
{
    if (m_loaderLoading == loading)
        return;
    m_loaderLoading = loading;
    emit loaderLoadingChanged();
}

bool AppController::isLoaderLoading() const
{
    return m_loaderLoading;
}

void AppController::tryFinishManifest()
{
    if (!m_bmclapiDone || !m_mojangDone)
        return;

    QString error;
    QVector<VersionInfo> bmclapi;
    QVector<VersionInfo> mojang;
    if (!m_bmclapiBody.isEmpty())
        bmclapi = VersionManifest::parse(m_bmclapiBody, &error);
    if (!m_mojangBody.isEmpty())
        mojang = VersionManifest::parse(m_mojangBody, &error);

    QVector<VersionInfo> merged;
    if (!bmclapi.isEmpty() && !mojang.isEmpty())
        merged = VersionManifest::merge(bmclapi, mojang);
    else if (!bmclapi.isEmpty())
        merged = bmclapi;
    else
        merged = mojang;

    if (merged.isEmpty())
    {
        setStatus(QStringLiteral("版本列表获取失败"));
        return;
    }

    setVersions(merged);
    setStatus(QStringLiteral("已加载 %1 个版本").arg(merged.size()));
}

void AppController::handleInstallProgress(qint64 received, qint64 total, const QString& currentFile)
{
    const int percent = total > 0 ? qBound(0, int((received * 100) / total), 100) : 0;
    if (m_progress != percent)
    {
        m_progress = percent;
        emit progressChanged();
    }
    if (!currentFile.isEmpty())
        setStatus(QStringLiteral("正在下载：%1").arg(QFileInfo(currentFile).fileName()));
}

void AppController::handleInstallFinished(const QString& versionId)
{
    m_installing = false;
    m_progress = 100;
    emit installingChanged();
    emit progressChanged();
    setStatus(QStringLiteral("安装完成：%1").arg(versionId));
}

void AppController::handleInstallFailed(const QString& message)
{
    m_installing = false;
    emit installingChanged();
    setStatus(QStringLiteral("安装失败：%1").arg(message));
    emit error(message);
}

void AppController::handleInstallCanceled()
{
    m_installing = false;
    emit installingChanged();
    setStatus(QStringLiteral("已取消"));
}
