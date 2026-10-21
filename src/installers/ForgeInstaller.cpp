#include "installers/ForgeInstaller.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>
#include <QVersionNumber>

#include "core/Logger.h"
#include "installers/JavaLocator.h"
#include "installers/ProcessRunner.h"
#include "installers/StagingArea.h"

namespace
{
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

    *error = lastError.isEmpty() ? QStringLiteral("无法下载 Forge 安装器") : lastError;
    return false;
}

QString findGeneratedVersion(const QString& stagingMinecraft, const QString& baseVersion, QString* error)
{
    const QDir versions(QDir(stagingMinecraft).filePath(QStringLiteral("versions")));
    const QStringList names = versions.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& name : names)
    {
        if (name == baseVersion)
            continue;

        const QString jsonPath = versions.filePath(QStringLiteral("%1/%1.json").arg(name));
        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        if (!QJsonDocument::fromJson(file.readAll()).isObject())
            continue;

        if (error != nullptr)
            error->clear();
        return name;
    }

    *error = QStringLiteral("Forge 安装器没有生成版本 JSON");
    return QString();
}
}

bool ForgeInstaller::isLegacy(const QString& mcVersion)
{
    const QVersionNumber version = QVersionNumber::fromString(mcVersion);
    if (version.isNull())
        return false;
    return version.majorVersion() == 1 && version.minorVersion() < 13;
}

QList<QUrl> ForgeInstaller::installerUrls(const QString& mcVersion, const QString& forgeVersion)
{
    const QString artifact = QStringLiteral("%1-%2").arg(mcVersion, forgeVersion);
    const QString path = QStringLiteral("net/minecraftforge/forge/%1/forge-%1-installer.jar").arg(artifact);
    return {
        QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/maven/") + path),
        QUrl(QStringLiteral("https://maven.minecraftforge.net/") + path)
    };
}

ForgeInstallResult ForgeInstaller::installModern(const QString& minecraftDir,
                                                  const QString& mcVersion,
                                                  const QString& forgeVersion,
                                                  const QString& javaPath,
                                                  const QJsonObject& versionJson)
{
    ForgeInstallResult result;

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

    const QString installer = QDir(staging.path()).filePath(QStringLiteral("forge-installer.jar"));
    if (!downloadFile(installerUrls(mcVersion, forgeVersion), installer, &error))
        return fail(error);

    QString java = javaPath;
    if (java.isEmpty())
    {
        int requiredMajor = versionJson.value(QStringLiteral("javaVersion")).toObject()
                                .value(QStringLiteral("majorVersion")).toInt();
        if (requiredMajor <= 0)
            requiredMajor = 8;

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
        QStringLiteral("--installClient"),
        staging.minecraftPath(),
        QStringLiteral("--mirror"),
        QStringLiteral("https://bmclapi2.bangbang93.com/maven")
    };

    ProcessResult process = ProcessRunner::run(java, args, QProcessEnvironment(), 15 * 60 * 1000);
    if (process.timedOut)
        return fail(QStringLiteral("Forge 安装器超时"));

    QString generated = findGeneratedVersion(staging.minecraftPath(), mcVersion, &error);
    if (generated.isEmpty())
    {
        // 镜像失败时按设计回退官方源再试一次。
        args = args.mid(0, args.size() - 2);
        process = ProcessRunner::run(java, args, QProcessEnvironment(), 15 * 60 * 1000);
        generated = findGeneratedVersion(staging.minecraftPath(), mcVersion, &error);
    }

    if (generated.isEmpty())
    {
        Logger::error(QStringLiteral("forge"),
                      QStringLiteral("安装器退出码 %1\n%2").arg(process.exitCode).arg(process.stdErr.right(4000)));
        return fail(error);
    }

    // 不让 staging 里的基底版本覆盖真实基底版本。
    QDir(QDir(staging.minecraftPath()).filePath(QStringLiteral("versions/%1").arg(mcVersion))).removeRecursively();

    if (!staging.mergeInto(&error))
        return fail(error);

    staging.cleanup();
    result.ok = true;
    result.versionId = generated;
    return result;
}
