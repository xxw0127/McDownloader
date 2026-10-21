#include "installers/NeoForgeInstaller.h"

#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>
#include <QVersionNumber>

#include "core/Checksum.h"
#include "core/Logger.h"
#include "core/Paths.h"
#include "installers/JavaLocator.h"
#include "installers/LoaderCatalog.h"
#include "installers/ProcessRunner.h"
#include "installers/ProfileInstaller.h"
#include "installers/StagingArea.h"
#include "installers/ZipExtractor.h"

namespace
{
QString normalizedLoaderVersion(const QString& mcVersion, const QString& loaderVersion)
{
    if (mcVersion == QLatin1String("1.20.1") && !loaderVersion.startsWith(QLatin1String("1.20.1-")))
        return QStringLiteral("1.20.1-") + loaderVersion;
    return loaderVersion;
}

bool copyDirectory(const QString& source, const QString& target, QString* error)
{
    QDir sourceDir(source);
    if (!sourceDir.exists())
        return true;

    QDirIterator it(source, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString sourceFile = it.next();
        const QString relative = sourceDir.relativeFilePath(sourceFile);
        const QString targetFile = QDir(target).filePath(relative);
        if (!QDir().mkpath(QFileInfo(targetFile).absolutePath()))
        {
            *error = QStringLiteral("无法创建库目录：%1").arg(QFileInfo(targetFile).absolutePath());
            return false;
        }
        QFile::remove(targetFile);
        if (!QFile::copy(sourceFile, targetFile))
        {
            *error = QStringLiteral("复制库失败：%1").arg(targetFile);
            return false;
        }
    }
    return true;
}

bool copyFile(const QString& source, const QString& target, QString* error)
{
    if (!QFileInfo::exists(source))
    {
        *error = QStringLiteral("文件不存在：%1").arg(source);
        return false;
    }
    if (!QDir().mkpath(QFileInfo(target).absolutePath()))
    {
        *error = QStringLiteral("无法创建目录：%1").arg(QFileInfo(target).absolutePath());
        return false;
    }
    QFile::remove(target);
    if (!QFile::copy(source, target))
    {
        *error = QStringLiteral("复制文件失败：%1").arg(target);
        return false;
    }
    return true;
}

bool writeLauncherProfiles(const QString& minecraftDir, QString* error)
{
    const QString target = QDir(minecraftDir).filePath(QStringLiteral("launcher_profiles.json"));
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

bool downloadFile(const QList<QUrl>& urls, const QString& target, QString* error)
{
    if (!QDir().mkpath(QFileInfo(target).absolutePath()))
    {
        *error = QStringLiteral("无法创建下载目录：%1").arg(QFileInfo(target).absolutePath());
        return false;
    }

    QNetworkAccessManager manager;
    QString lastError;
    for (const QUrl& url : urls)
    {
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PCL2Downloader/0.1"));

        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(120000, &loop, &QEventLoop::quit);
        if (!reply->isFinished())
            loop.exec();

        if (!reply->isFinished())
        {
            reply->abort();
            lastError = QStringLiteral("下载超时：%1").arg(url.toString());
            reply->deleteLater();
            continue;
        }

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const QNetworkReply::NetworkError netError = reply->error();
        const QString netText = reply->errorString();
        reply->deleteLater();

        if (netError != QNetworkReply::NoError || status < 200 || status >= 300 || body.isEmpty())
        {
            lastError = QStringLiteral("%1（HTTP %2，%3）").arg(url.toString()).arg(status).arg(netText);
            continue;
        }

        QSaveFile file(target);
        if (!file.open(QIODevice::WriteOnly) || file.write(body) != body.size() || !file.commit())
        {
            lastError = QStringLiteral("写入安装器失败：%1").arg(target);
            continue;
        }
        return true;
    }

    *error = lastError.isEmpty() ? QStringLiteral("无法下载 NeoForge 安装器") : lastError;
    return false;
}

bool prepareInstallerLibraries(const QString& installerPath,
                               const QString& stagingMinecraft,
                               QString* error)
{
    ZipExtractor zip(installerPath);
    if (!zip.isValid())
    {
        *error = QStringLiteral("无法打开 NeoForge 安装器 ZIP");
        return false;
    }

    const QByteArray raw = zip.readFile(QStringLiteral("install_profile.json"), error);
    if (error != nullptr && !error->isEmpty())
        return false;

    const QJsonDocument document = QJsonDocument::fromJson(raw);
    if (!document.isObject())
    {
        *error = QStringLiteral("NeoForge install_profile.json 无效");
        return false;
    }

    const InstallPlan plan = ProfileInstaller::buildPlan(document.object(),
                                                          stagingMinecraft,
                                                          QStringLiteral("neoforge-installer"));
    for (const FileSpec& file : plan.files)
    {
        bool ok = false;
        const QString target = Paths::joinWithin(stagingMinecraft, file.targetRelativePath, &ok);
        if (!ok)
        {
            *error = QStringLiteral("安装器库路径不安全：%1").arg(file.targetRelativePath);
            return false;
        }

        QString verifyError;
        if (Checksum::verifyFile(target, file.size, file.sha1, &verifyError))
            continue;

        if (file.urls.isEmpty())
        {
            *error = QStringLiteral("安装器库缺少下载地址：%1").arg(file.targetRelativePath);
            return false;
        }

        if (!downloadFile(file.urls, target, error))
            return false;

        if (!Checksum::verifyFile(target, file.size, file.sha1, &verifyError))
        {
            QFile::remove(target);
            *error = verifyError;
            return false;
        }
    }
    return true;
}

bool patchGeneratedVersion(const QString& stagingMinecraft,
                           const QString& mcVersion,
                           const QString& loaderVersion,
                           const QString& generatedVersion,
                           QString* error)
{
    Q_UNUSED(mcVersion)

    const QString jsonPath = QDir(stagingMinecraft)
                                 .filePath(QStringLiteral("versions/%1/%1.json").arg(generatedVersion));
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        *error = QStringLiteral("无法读取 NeoForge 版本 JSON：%1").arg(jsonPath);
        return false;
    }

    QJsonObject versionJson = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    if (versionJson.isEmpty())
    {
        *error = QStringLiteral("NeoForge 版本 JSON 无效");
        return false;
    }

    // NeoForge 自带 client provider，不能再用父版本 obfuscated jar，否则模块冲突。
    versionJson.remove(QStringLiteral("jar"));

    QJsonArray libraries = versionJson.value(QStringLiteral("libraries")).toArray();
    const bool legacyForgeArtifact = (mcVersion == QLatin1String("1.20.1"));
    bool hasMainLibrary = false;
    for (const QJsonValue& value : libraries)
    {
        const QString name = value.toObject().value(QStringLiteral("name")).toString();
        if (name.startsWith(QLatin1String("net.neoforged:neoforge:")) ||
            (legacyForgeArtifact && name.startsWith(QLatin1String("net.neoforged:forge:"))))
        {
            hasMainLibrary = true;
            break;
        }
    }

    if (!hasMainLibrary)
    {
        const QString fullVersion = normalizedLoaderVersion(mcVersion, loaderVersion);
        const QString kind = NeoForgeInstaller::artifactKind(mcVersion);
        const QString path = legacyForgeArtifact
                                 ? QStringLiteral("net/neoforged/forge/%1/forge-%1-universal.jar").arg(fullVersion)
                                 : QStringLiteral("net/neoforged/neoforge/%1/neoforge-%1-universal.jar").arg(fullVersion);
        const QString relative = QStringLiteral("libraries/") + path;
        bool ok = false;
        const QString target = Paths::joinWithin(stagingMinecraft, relative, &ok);
        if (!ok)
        {
            *error = QStringLiteral("NeoForge 主模块路径不安全");
            return false;
        }

        if (!QFileInfo::exists(target))
        {
            const QList<QUrl> urls = {
                QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/maven/") + path),
                QUrl(QStringLiteral("https://maven.neoforged.net/releases/") + path)
            };
            if (!downloadFile(urls, target, error))
                return false;
        }

        const QByteArray sha1 = Checksum::sha1File(target, error);
        if (error != nullptr && !error->isEmpty())
            return false;
        const qint64 size = QFileInfo(target).size();

        QJsonObject artifact;
        artifact.insert(QStringLiteral("path"), path);
        artifact.insert(QStringLiteral("url"),
                        QStringLiteral("https://maven.neoforged.net/releases/") + path);
        artifact.insert(QStringLiteral("sha1"), QString::fromLatin1(sha1.toHex()));
        artifact.insert(QStringLiteral("size"), size);

        QJsonObject downloads;
        downloads.insert(QStringLiteral("artifact"), artifact);
        QJsonObject library;
        library.insert(QStringLiteral("name"),
                       QStringLiteral("net.neoforged:%1:%2:universal").arg(kind, fullVersion));
        library.insert(QStringLiteral("downloads"), downloads);
        libraries.append(library);
        versionJson.insert(QStringLiteral("libraries"), libraries);
    }

    QSaveFile output(jsonPath);
    if (!output.open(QIODevice::WriteOnly))
    {
        *error = QStringLiteral("无法写入 NeoForge 版本 JSON");
        return false;
    }
    output.write(QJsonDocument(versionJson).toJson(QJsonDocument::Indented));
    if (!output.commit())
    {
        *error = QStringLiteral("提交 NeoForge 版本 JSON 失败");
        return false;
    }
    return true;
}

QString findGeneratedVersion(const QString& stagingMinecraft, const QString& baseVersion, QString* error)
{
    const QDir versions(QDir(stagingMinecraft).filePath(QStringLiteral("versions")));
    for (const QString& name : versions.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (name == baseVersion)
            continue;
        const QString jsonPath = versions.filePath(QStringLiteral("%1/%1.json").arg(name));
        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly))
            continue;
        if (QJsonDocument::fromJson(file.readAll()).isObject())
        {
            if (error != nullptr)
                error->clear();
            return name;
        }
    }
    *error = QStringLiteral("NeoForge 安装器没有生成版本 JSON");
    return QString();
}
}

bool NeoForgeInstaller::isSupported(const QString& mcVersion)
{
    const QVersionNumber version = QVersionNumber::fromString(mcVersion);
    return !version.isNull() && QVersionNumber::compare(version, QVersionNumber(1, 20, 1)) >= 0;
}

QString NeoForgeInstaller::artifactKind(const QString& mcVersion)
{
    return mcVersion == QLatin1String("1.20.1") ? QStringLiteral("forge") : QStringLiteral("neoforge");
}

QString NeoForgeInstaller::instanceId(const QString& mcVersion, const QString& loaderVersion)
{
    if (mcVersion == QLatin1String("1.20.1"))
    {
        QString version = loaderVersion;
        if (version.contains(QLatin1Char('-')))
            version = version.section(QLatin1Char('-'), 1);
        return QStringLiteral("1.20.1-forge-%1").arg(version);
    }

    // NeoForge 1.20.2+ 安装器生成的版本 id 形如 neoforge-21.1.80。
    return QStringLiteral("neoforge-%1").arg(loaderVersion);
}

QList<QUrl> NeoForgeInstaller::installerUrls(const QString& mcVersion, const QString& loaderVersion)
{
    const QString artifact = normalizedLoaderVersion(mcVersion, loaderVersion);
    const QString kind = artifactKind(mcVersion);
    const QString path = QStringLiteral("net/neoforged/%1/%2/%1-%2-installer.jar").arg(kind, artifact);
    return {
        QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/maven/") + path),
        QUrl(QStringLiteral("https://maven.neoforged.net/releases/") + path)
    };
}

QVector<LoaderVersion> NeoForgeInstaller::parseMavenVersions(const QByteArray& body,
                                                             const QString& mcVersion,
                                                             QString* error)
{
    return LoaderCatalog::parseIndex(LoaderKind::NeoForge, body, mcVersion, error);
}

NeoForgeInstallResult NeoForgeInstaller::install(const QString& minecraftDir,
                                                  const QString& mcVersion,
                                                  const QString& loaderVersion,
                                                  const QString& javaPath,
                                                  const QJsonObject& versionJson,
                                                  const std::atomic_bool* cancelFlag)
{
    NeoForgeInstallResult result;
    QString error;
    StagingArea staging = StagingArea::create(minecraftDir, &error);
    if (staging.path().isEmpty())
    {
        result.error = error;
        return result;
    }

    auto fail = [&](const QString& message) {
        staging.cleanup();
        result.error = message;
        return result;
    };

    const QDir baseDir(QDir(minecraftDir).filePath(QStringLiteral("versions/%1").arg(mcVersion)));
    if (!baseDir.exists())
        return fail(QStringLiteral("原版版本目录不存在：%1").arg(mcVersion));

    const QDir stageMc(staging.minecraftPath());
    const QString stageVersionDir = stageMc.filePath(QStringLiteral("versions/%1").arg(mcVersion));
    if (!copyFile(baseDir.filePath(QStringLiteral("%1.json").arg(mcVersion)),
                  QDir(stageVersionDir).filePath(QStringLiteral("%1.json").arg(mcVersion)), &error) ||
        !copyFile(baseDir.filePath(QStringLiteral("%1.jar").arg(mcVersion)),
                  QDir(stageVersionDir).filePath(QStringLiteral("%1.jar").arg(mcVersion)), &error))
    {
        return fail(error);
    }

    if (!writeLauncherProfiles(staging.minecraftPath(), &error))
        return fail(error);

    const QString installer = QDir(staging.path()).filePath(QStringLiteral("neoforge-installer.jar"));
    if (!downloadFile(installerUrls(mcVersion, loaderVersion), installer, &error))
        return fail(error);

    // 安装器会重新检查全部库；把已有库复制进 staging，避免官方 Maven 超时。
    const QString existingLibraries = QDir(minecraftDir).filePath(QStringLiteral("libraries"));
    if (QDir(existingLibraries).exists())
    {
        if (!copyDirectory(existingLibraries,
                           QDir(staging.minecraftPath()).filePath(QStringLiteral("libraries")),
                           &error))
            return fail(error);
    }

    // NeoForge 安装器的 --mirror 不会覆盖处理器库；先把 install_profile 的库全部下好。
    if (!prepareInstallerLibraries(installer, staging.minecraftPath(), &error))
        return fail(error);

    QString java = javaPath;
    if (java.isEmpty())
    {
        int requiredMajor = versionJson.value(QStringLiteral("javaVersion")).toObject()
                                .value(QStringLiteral("majorVersion")).toInt();
        if (requiredMajor <= 0)
            requiredMajor = 21;
        const QVector<JavaInfo> candidates = JavaLocator::scan(minecraftDir);
        const JavaInfo selected = JavaLocator::selectForInstaller(candidates, requiredMajor);
        if (selected.path.isEmpty())
            return fail(QStringLiteral("找不到满足 Java %1 的 64 位 Java").arg(requiredMajor));
        java = selected.path;
    }

    QStringList args = {
        QStringLiteral("-Djava.awt.headless=true"),
        QStringLiteral("-jar"),
        installer,
        QStringLiteral("--install-client"),
        staging.minecraftPath(),
        QStringLiteral("--mirror"),
        QStringLiteral("https://bmclapi2.bangbang93.com/maven")
    };

    ProcessResult process = ProcessRunner::run(java, args, QProcessEnvironment(), 15 * 60 * 1000, cancelFlag);
    if (process.canceled)
        return fail(QStringLiteral("canceled"));
    if (process.timedOut)
        return fail(QStringLiteral("NeoForge 安装器超时"));

    QString generated = findGeneratedVersion(staging.minecraftPath(), mcVersion, &error);
    if (process.exitCode != 0 || generated.isEmpty())
    {
        args = args.mid(0, args.size() - 2);
        process = ProcessRunner::run(java, args, QProcessEnvironment(), 15 * 60 * 1000, cancelFlag);
        generated = findGeneratedVersion(staging.minecraftPath(), mcVersion, &error);
    }

    if (process.canceled)
        return fail(QStringLiteral("canceled"));
    if (generated.isEmpty() || process.exitCode != 0)
    {
        Logger::error(QStringLiteral("neoforge"),
                      QStringLiteral("安装器退出码 %1\n%2").arg(process.exitCode).arg(process.stdErr.right(4000)));
        return fail(generated.isEmpty() ? error
                                        : QStringLiteral("NeoForge 安装器退出码 %1").arg(process.exitCode));
    }

    if (!patchGeneratedVersion(staging.minecraftPath(), mcVersion, loaderVersion, generated, &error))
        return fail(error);

    if (cancelFlag != nullptr && cancelFlag->load())
        return fail(QStringLiteral("canceled"));

    if (!QDir(QDir(staging.minecraftPath()).filePath(QStringLiteral("versions/%1").arg(mcVersion))).removeRecursively())
        return fail(QStringLiteral("无法清理 staging 基底版本"));
    if (!staging.mergeInto(&error))
        return fail(error);

    staging.cleanup();
    result.ok = true;
    result.versionId = generated;
    return result;
}
