#include "tst_mirror.h"

#include "download/MirrorResolver.h"

void TstMirror::manifestUrlsOrder()
{
    const QList<QUrl> urls = MirrorResolver::manifestUrls();
    QCOMPARE(urls.size(), 2);
    QCOMPARE(urls.at(0).host(), QStringLiteral("bmclapi2.bangbang93.com"));
    QCOMPARE(urls.at(1).host(), QStringLiteral("launchermeta.mojang.com"));
}

void TstMirror::assetUrlsOrder()
{
    const QList<QUrl> urls = MirrorResolver::assetUrls(QStringLiteral("aabbcc"));
    QCOMPARE(urls.size(), 2);
    QCOMPARE(urls.at(0).toString(), QStringLiteral("https://bmclapi2.bangbang93.com/assets/aa/aabbcc"));
    QCOMPARE(urls.at(1).toString(), QStringLiteral("https://resources.download.minecraft.net/aa/aabbcc"));
}

void TstMirror::metaUrlMirrorsPistonMeta()
{
    const QUrl official(QStringLiteral("https://piston-meta.mojang.com/v1/packages/abc/1.20.1.json"));
    const QList<QUrl> urls = MirrorResolver::metaUrls(official);
    QCOMPARE(urls.size(), 2);
    QCOMPARE(urls.at(0).toString(), QStringLiteral("https://bmclapi2.bangbang93.com/v1/packages/abc/1.20.1.json"));
    QCOMPARE(urls.at(1), official);
}

void TstMirror::libraryUrlsPreferBmclapi()
{
    const QUrl official(QStringLiteral("https://libraries.minecraft.net/net/minecraft/launchwrapper/1.12/launchwrapper-1.12.jar"));
    const QList<QUrl> urls = MirrorResolver::libraryUrls(QStringLiteral("net/minecraft/launchwrapper/1.12/launchwrapper-1.12.jar"), official);
    QCOMPARE(urls.size(), 2);
    QCOMPARE(urls.at(0).toString(), QStringLiteral("https://bmclapi2.bangbang93.com/maven/net/minecraft/launchwrapper/1.12/launchwrapper-1.12.jar"));
    QCOMPARE(urls.at(1), official);
}

void TstMirror::quiltLibraryUrlsPreferOfficial()
{
    const QUrl official(QStringLiteral("https://maven.quiltmc.org/repository/release/org/quiltmc/quilt-loader/0.23.1/quilt-loader-0.23.1.jar"));
    const QList<QUrl> urls = MirrorResolver::libraryUrls(QStringLiteral("org/quiltmc/quilt-loader/0.23.1/quilt-loader-0.23.1.jar"), official);
    QCOMPARE(urls.size(), 2);
    QCOMPARE(urls.at(0), official);
    QCOMPARE(urls.at(1).host(), QStringLiteral("bmclapi2.bangbang93.com"));
}
