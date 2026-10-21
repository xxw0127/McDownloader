#include "tst_vanilla.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/JsonRules.h"
#include "installers/AssetResolver.h"
#include "installers/InstallPlan.h"
#include "installers/LibraryResolver.h"
#include "installers/VanillaPlanBuilder.h"

namespace
{
QJsonObject loadObject(const QString& relativePath)
{
    QFile file(QStringLiteral(TEST_FIXTURE_DIR "/") + relativePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.object();
}

bool containsTarget(const QVector<FileSpec>& files, const QString& target)
{
    for (const FileSpec& file : files)
    {
        if (file.targetRelativePath == target)
            return true;
    }
    return false;
}
}

void TstVanilla::rulesDefaultDeny()
{
    const QJsonArray windowsRule = {QJsonObject{{QStringLiteral("action"), QStringLiteral("allow")},
                                                {QStringLiteral("os"), QJsonObject{{QStringLiteral("name"), QStringLiteral("windows")}}}}};
    const QJsonArray osxRule = {QJsonObject{{QStringLiteral("action"), QStringLiteral("allow")},
                                            {QStringLiteral("os"), QJsonObject{{QStringLiteral("name"), QStringLiteral("osx")}}}}};

    QVERIFY(JsonRules::allowed(windowsRule, QStringLiteral("windows"), QStringLiteral("x64")));
    QVERIFY(!JsonRules::allowed(osxRule, QStringLiteral("windows"), QStringLiteral("x64")));
    QVERIFY(JsonRules::allowed(QJsonArray(), QStringLiteral("windows"), QStringLiteral("x64")));
}

void TstVanilla::windowsNativesSelected()
{
    const QJsonObject version = loadObject(QStringLiteral("version_1_20_1_snippet.json"));
    const QVector<LibraryFile> libraries = LibraryResolver::resolve(version, QStringLiteral("windows"), QStringLiteral("x64"));

    bool foundNative = false;
    bool foundOsx = false;
    bool foundLaunchwrapper = false;
    for (const LibraryFile& library : libraries)
    {
        if (library.relativePath.contains(QStringLiteral("native-1.0-natives-windows.jar")))
        {
            foundNative = true;
            QVERIFY(library.isNatives);
        }
        if (library.relativePath.contains(QStringLiteral("osx-only")))
            foundOsx = true;
        if (library.relativePath.contains(QStringLiteral("launchwrapper-1.12.jar")))
            foundLaunchwrapper = true;
    }

    QVERIFY(foundNative);
    QVERIFY(!foundOsx);
    QVERIFY(foundLaunchwrapper);
}

void TstVanilla::mavenFallbackPath()
{
    const QJsonObject version = loadObject(QStringLiteral("version_1_20_1_snippet.json"));
    const QVector<LibraryFile> libraries = LibraryResolver::resolve(version, QStringLiteral("windows"), QStringLiteral("x64"));

    bool found = false;
    for (const LibraryFile& library : libraries)
    {
        if (library.relativePath == QStringLiteral("libraries/net/minecraft/launchwrapper/1.12/launchwrapper-1.12.jar"))
        {
            found = true;
            QVERIFY(!library.urls.isEmpty());
            QVERIFY(library.urls.first().toString().contains(QStringLiteral("launchwrapper-1.12.jar")));
        }
    }
    QVERIFY(found);
}

void TstVanilla::assetObjectAndLegacyPaths()
{
    const QJsonObject index = QJsonDocument::fromJson(R"({
        "virtual": true,
        "map_to_resources": true,
        "objects": {
            "minecraft/sounds/click.ogg": {
                "hash": "aabbccddeeff00112233445566778899aabbccdd",
                "size": 321
            }
        }
    })").object();

    const QVector<AssetFile> assets = AssetResolver::parseIndex(index, QStringLiteral("1.7.10"));
    QCOMPARE(assets.size(), 1);
    QCOMPARE(assets.first().relativePath,
             QStringLiteral("assets/objects/aa/aabbccddeeff00112233445566778899aabbccdd"));
    QCOMPARE(assets.first().legacyPath, QStringLiteral("minecraft/sounds/click.ogg"));
    QVERIFY(assets.first().virtualAsset);
    QVERIFY(assets.first().mapToResources);
    QCOMPARE(assets.first().size, 321);
}

void TstVanilla::loggingConfigIncluded()
{
    const QJsonObject version = loadObject(QStringLiteral("version_1_20_1_snippet.json"));
    const InstallPlan plan = VanillaPlanBuilder::build(version, QStringLiteral("D:/mc"), QStringLiteral("1.20.1"));
    QVERIFY(containsTarget(plan.files, QStringLiteral("assets/log_configs/client-1.12.xml")));
}

void TstVanilla::clientAndAssetIndexIncluded()
{
    const QJsonObject version = loadObject(QStringLiteral("version_1_20_1_snippet.json"));
    const InstallPlan plan = VanillaPlanBuilder::build(version, QStringLiteral("D:/mc"), QStringLiteral("1.20.1"));

    QVERIFY(containsTarget(plan.files, QStringLiteral("versions/1.20.1/1.20.1.jar")));
    QVERIFY(containsTarget(plan.files, QStringLiteral("assets/indexes/5.json")));
    QCOMPARE(plan.assetIndexId, QStringLiteral("5"));
    QVERIFY(plan.files.size() >= 6);
}

void TstVanilla::legacyWithoutClientDoesNotCrash()
{
    const QJsonObject version = loadObject(QStringLiteral("version_legacy_snippet.json"));
    const InstallPlan plan = VanillaPlanBuilder::build(version, QStringLiteral("D:/mc"), QStringLiteral("1.7.10"));

    QCOMPARE(plan.versionId, QStringLiteral("1.7.10"));
    QCOMPARE(plan.assetIndexId, QStringLiteral("legacy"));
    QVERIFY(plan.jarPath.isEmpty());
}
