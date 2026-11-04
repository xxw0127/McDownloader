#include "tst_forge.h"

#include "installers/ForgeInstaller.h"

void TstForge::legacyDetection()
{
    QVERIFY(ForgeInstaller::isLegacy(QStringLiteral("1.7.10")));
    QVERIFY(ForgeInstaller::isLegacy(QStringLiteral("1.12.2")));
    QVERIFY(!ForgeInstaller::isLegacy(QStringLiteral("1.13")));
    QVERIFY(!ForgeInstaller::isLegacy(QStringLiteral("1.20.1")));
}

void TstForge::installerUrls()
{
    const QList<QUrl> urls = ForgeInstaller::installerUrls(QStringLiteral("1.20.1"), QStringLiteral("47.2.0"));
    QCOMPARE(urls.size(), 2);
    QVERIFY(urls.first().toString().contains(QStringLiteral("forge-1.20.1-47.2.0-installer.jar")));
    QCOMPARE(urls.first().host(), QStringLiteral("bmclapi2.bangbang93.com"));
    QCOMPARE(urls.last().host(), QStringLiteral("maven.minecraftforge.net"));
}
