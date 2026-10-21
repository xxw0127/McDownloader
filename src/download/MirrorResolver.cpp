#include "download/MirrorResolver.h"

namespace
{
const char* const kBmclapiHost = "bmclapi2.bangbang93.com";
const char* const kMojangManifest = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
const char* const kBmclapiManifest = "https://bmclapi2.bangbang93.com/mc/game/version_manifest.json";
const char* const kOfficialAssets = "https://resources.download.minecraft.net/";
const char* const kBmclapiAssets = "https://bmclapi2.bangbang93.com/assets/";
const char* const kBmclapiMavenBase = "https://bmclapi2.bangbang93.com/maven/";

QString trimLeadingSlash(QString path)
{
    while (path.startsWith(QLatin1Char('/')))
        path.remove(0, 1);
    return path;
}
}

QList<QUrl> MirrorResolver::manifestUrls()
{
    return {QUrl(QString::fromLatin1(kBmclapiManifest)), QUrl(QString::fromLatin1(kMojangManifest))};
}

QList<QUrl> MirrorResolver::metaUrls(const QUrl& official)
{
    QUrl mirrored = official;
    const QString host = official.host().toLower();
    if (host == QLatin1String("piston-meta.mojang.com") ||
        host == QLatin1String("piston-data.mojang.com") ||
        host == QLatin1String("launcher.mojang.com") ||
        host == QLatin1String("launchermeta.mojang.com"))
    {
        mirrored.setScheme(QStringLiteral("https"));
        mirrored.setHost(QString::fromLatin1(kBmclapiHost));
        mirrored.setPort(-1);
    }

    if (mirrored == official)
        return {official};
    return {mirrored, official};
}

QList<QUrl> MirrorResolver::assetUrls(const QString& hash)
{
    const QString prefix = hash.left(2);
    const QString suffix = QStringLiteral("%1/%2").arg(prefix, hash);
    return {QUrl(QString::fromLatin1(kBmclapiAssets) + suffix),
            QUrl(QString::fromLatin1(kOfficialAssets) + suffix)};
}

QList<QUrl> MirrorResolver::libraryUrls(const QString& mavenPath, const QUrl& official)
{
    const QString path = normalizedMavenPath(mavenPath, official);
    const QUrl mirror = bmclapiMaven(path);
    const QString host = official.host().toLower();

    if (host.contains(QLatin1String("quiltmc")))
        return {official, mirror};

    if (host == QLatin1String("libraries.minecraft.net") ||
        host == QLatin1String("maven.minecraftforge.net") ||
        host == QLatin1String("files.minecraftforge.net") ||
        host == QLatin1String("maven.fabricmc.net") ||
        host == QLatin1String("maven.neoforged.net") ||
        host.isEmpty())
    {
        return {mirror, official};
    }

    return {official};
}

QUrl MirrorResolver::bmclapiMaven(const QString& mavenPath)
{
    return QUrl(QString::fromLatin1(kBmclapiMavenBase) + trimLeadingSlash(mavenPath));
}

QString MirrorResolver::normalizedMavenPath(const QString& mavenPath, const QUrl& official)
{
    if (!mavenPath.trimmed().isEmpty())
        return trimLeadingSlash(mavenPath.trimmed());

    QString path = trimLeadingSlash(official.path());
    const QString host = official.host().toLower();
    if (host == QLatin1String("maven.neoforged.net") && path.startsWith(QLatin1String("releases/")))
        path = path.mid(int(qstrlen("releases/")));
    return path;
}
