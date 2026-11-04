#include "tst_neoforge.h"

#include "installers/NeoForgeInstaller.h"

void TstNeoForge::supportMatrix()
{
    QVERIFY(NeoForgeInstaller::isSupported(QStringLiteral("1.20.1")));
    QVERIFY(NeoForgeInstaller::isSupported(QStringLiteral("1.21.1")));
    QVERIFY(!NeoForgeInstaller::isSupported(QStringLiteral("1.19.4")));
}

void TstNeoForge::artifactKinds()
{
    QCOMPARE(NeoForgeInstaller::artifactKind(QStringLiteral("1.20.1")), QStringLiteral("forge"));
    QCOMPARE(NeoForgeInstaller::artifactKind(QStringLiteral("1.21.1")), QStringLiteral("neoforge"));
}

void TstNeoForge::instanceIds()
{
    QCOMPARE(NeoForgeInstaller::instanceId(QStringLiteral("1.20.1"), QStringLiteral("47.1.106")),
             QStringLiteral("1.20.1-forge-47.1.106"));
    QCOMPARE(NeoForgeInstaller::instanceId(QStringLiteral("1.21.1"), QStringLiteral("21.1.80")),
             QStringLiteral("neoforge-21.1.80"));
}

void TstNeoForge::parsesMavenVersions1201()
{
    const QByteArray body = R"({
        "isSnapshot": false,
        "versions": ["1.20.1-47.1.5", "1.20.1-47.1.106", "1.20.1-47.1.105", "1.21.1-47.1.1"]
    })";
    QString error;
    const QVector<LoaderVersion> versions = NeoForgeInstaller::parseMavenVersions(
        body, QStringLiteral("1.20.1"), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(versions.size(), 3);
    QCOMPARE(versions.first().version, QStringLiteral("1.20.1-47.1.106"));
}

void TstNeoForge::parsesMavenVersions1211()
{
    const QByteArray body = R"({
        "isSnapshot": false,
        "versions": ["21.1.80", "21.1.79", "21.0.1", "20.6.1"]
    })";
    QString error;
    const QVector<LoaderVersion> versions = NeoForgeInstaller::parseMavenVersions(
        body, QStringLiteral("1.21.1"), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(versions.size(), 2);
    QCOMPARE(versions.first().version, QStringLiteral("21.1.80"));
}

void TstNeoForge::installerUrls()
{
    const QList<QUrl> modern = NeoForgeInstaller::installerUrls(QStringLiteral("1.21.1"),
                                                                QStringLiteral("21.1.80"));
    QCOMPARE(modern.size(), 2);
    QVERIFY(modern.first().toString().contains(QStringLiteral("neoforge-21.1.80-installer.jar")));
    QCOMPARE(modern.first().host(), QStringLiteral("bmclapi2.bangbang93.com"));

    const QList<QUrl> legacy = NeoForgeInstaller::installerUrls(QStringLiteral("1.20.1"),
                                                                QStringLiteral("47.1.106"));
    QVERIFY(legacy.first().toString().contains(QStringLiteral("forge-1.20.1-47.1.106-installer.jar")));
}
