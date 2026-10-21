#include "tst_loader.h"

#include <QJsonDocument>

#include "installers/LoaderCatalog.h"
#include "installers/ProfileInstaller.h"

void TstLoader::parsesFabricIndex()
{
    const QByteArray body = R"({
        "game": [{"version": "1.20.1", "stable": true}],
        "loader": [
            {"version": "0.15.11", "stable": true},
            {"version": "0.15.10", "stable": true}
        ]
    })";

    QString error;
    const QVector<LoaderVersion> versions = LoaderCatalog::parseIndex(LoaderKind::Fabric, body,
                                                                     QStringLiteral("1.20.1"), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(versions.size(), 2);
    QCOMPARE(versions.first().version, QStringLiteral("0.15.11"));
    QCOMPARE(versions.first().stable, QStringLiteral("stable"));
}

void TstLoader::rejectsUnsupportedFabricGame()
{
    const QByteArray body = R"({
        "game": [{"version": "1.20.2"}],
        "loader": [{"version": "0.15.11"}]
    })";

    QString error;
    const QVector<LoaderVersion> versions = LoaderCatalog::parseIndex(LoaderKind::Fabric, body,
                                                                     QStringLiteral("1.20.1"), &error);
    QVERIFY(versions.isEmpty());
    QVERIFY(!error.isEmpty());
}

void TstLoader::parsesQuiltIndex()
{
    const QByteArray body = R"([
        {"loader": {"version": "0.23.1", "build": 1}},
        {"loader": {"version": "0.23.0", "build": 2}}
    ])";

    QString error;
    const QVector<LoaderVersion> versions = LoaderCatalog::parseIndex(LoaderKind::Quilt, body,
                                                                     QStringLiteral("1.20.1"), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(versions.size(), 2);
    QCOMPARE(versions.first().version, QStringLiteral("0.23.1"));
}

void TstLoader::fabricProfileUrlKeepsBuildSuffix()
{
    const QList<QUrl> urls = LoaderCatalog::profileUrls(LoaderKind::Fabric,
                                                        QStringLiteral("1.20.1"),
                                                        QStringLiteral("0.15.11+build.1"));
    QVERIFY(!urls.isEmpty());
    const QString encoded = urls.first().toEncoded();
    QVERIFY(encoded.contains(QStringLiteral("0.15.11+build.1")) ||
            encoded.contains(QStringLiteral("0.15.11%2Bbuild.1")));
}

void TstLoader::quiltProfileUrlHost()
{
    const QList<QUrl> urls = LoaderCatalog::profileUrls(LoaderKind::Quilt,
                                                        QStringLiteral("1.20.1"),
                                                        QStringLiteral("0.23.1"));
    QVERIFY(!urls.isEmpty());
    QCOMPARE(urls.first().host(), QStringLiteral("meta.quiltmc.org"));
}

void TstLoader::instanceIds()
{
    QCOMPARE(ProfileInstaller::instanceId(LoaderKind::Fabric, QStringLiteral("1.20.1"), QStringLiteral("0.15.11")),
             QStringLiteral("fabric-loader-0.15.11-1.20.1"));
    QCOMPARE(ProfileInstaller::instanceId(LoaderKind::Quilt, QStringLiteral("1.20.1"), QStringLiteral("0.23.1")),
             QStringLiteral("quilt-loader-0.23.1-1.20.1"));
}

void TstLoader::prepareProfileOverridesId()
{
    const QByteArray raw = R"({
        "id": "fabric-loader-0.15.11-1.20.1",
        "inheritsFrom": "1.20.1",
        "libraries": []
    })";

    QString error;
    const QJsonObject profile = ProfileInstaller::prepareProfile(raw, QStringLiteral("custom-id"), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(profile.value(QStringLiteral("id")).toString(), QStringLiteral("custom-id"));
    QCOMPARE(profile.value(QStringLiteral("inheritsFrom")).toString(), QStringLiteral("1.20.1"));
}

void TstLoader::quiltLibraryPrefersOfficial()
{
    const QByteArray raw = R"({
        "id": "quilt-loader-0.20.0-beta.9-1.20.1",
        "inheritsFrom": "1.20.1",
        "libraries": [
            {
                "name": "org.quiltmc:hashed:1.20.1",
                "url": "https://maven.quiltmc.org/repository/release/"
            }
        ]
    })";

    QString error;
    const QJsonObject profile = ProfileInstaller::prepareProfile(raw, QStringLiteral("quilt-test"), &error);
    QVERIFY(error.isEmpty());
    const InstallPlan plan = ProfileInstaller::buildPlan(profile, QStringLiteral("D:/mc"), QStringLiteral("quilt-test"));
    QCOMPARE(plan.files.size(), 1);
    QCOMPARE(plan.files.first().urls.first().host(), QStringLiteral("maven.quiltmc.org"));
}

void TstLoader::buildPlanUsesMavenUrl()
{
    const QByteArray raw = R"({
        "id": "fabric-loader-0.15.11-1.20.1",
        "inheritsFrom": "1.20.1",
        "libraries": [
            {
                "name": "net.fabricmc:tiny-mappings-parser:0.3.0+build.17",
                "url": "https://maven.fabricmc.net/"
            }
        ]
    })";

    QString error;
    const QJsonObject profile = ProfileInstaller::prepareProfile(raw, QStringLiteral("fabric-loader-0.15.11-1.20.1"), &error);
    QVERIFY(error.isEmpty());

    const InstallPlan plan = ProfileInstaller::buildPlan(profile, QStringLiteral("D:/mc"),
                                                         QStringLiteral("fabric-loader-0.15.11-1.20.1"));
    QCOMPARE(plan.files.size(), 1);
    QCOMPARE(plan.files.first().targetRelativePath,
             QStringLiteral("libraries/net/fabricmc/tiny-mappings-parser/0.3.0+build.17/tiny-mappings-parser-0.3.0+build.17.jar"));
    QVERIFY(!plan.files.first().urls.isEmpty());
    QVERIFY(plan.files.first().urls.first().toString().contains(QStringLiteral("tiny-mappings-parser-0.3.0+build.17.jar")));
}
