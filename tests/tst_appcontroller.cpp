#include "tst_appcontroller.h"

#include <QDir>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "AppController.h"
#include "core/AppSettings.h"

void TstAppController::folderValidation()
{
    AppController controller;
    const QString original = controller.minecraftDir();

    controller.setMinecraftDir(QStringLiteral("Z:/definitely/not/here"));
    QCOMPARE(controller.minecraftDir(), original);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    controller.setMinecraftDir(dir.path());
    QCOMPARE(controller.minecraftDir(), QDir(dir.path()).absolutePath());
}

void TstAppController::instanceIdForVanilla()
{
    AppController controller;
    controller.setLoaderKind(QStringLiteral("vanilla"));
    controller.setSelectedVersion(QStringLiteral("1.20.1"));
    QCOMPARE(controller.instanceId(), QStringLiteral("1.20.1"));
}

void TstAppController::instanceIdForFabric()
{
    AppController controller;
    controller.setLoaderKind(QStringLiteral("fabric"));
    controller.setLoaderVersion(QStringLiteral("0.15.11"));
    controller.setSelectedVersion(QStringLiteral("1.20.1"));
    QCOMPARE(controller.instanceId(), QStringLiteral("fabric-loader-0.15.11-1.20.1"));
}

void TstAppController::startWithoutVersionEmitsError()
{
    AppController controller;
    controller.setSelectedVersion(QString());
    QSignalSpy error(&controller, &AppController::error);

    controller.startInstall();

    QCOMPARE(error.count(), 1);
    QVERIFY(!controller.isInstalling());
}

void TstAppController::loaderKindChangeClearsVersion()
{
    AppController controller;
    controller.setLoaderVersion(QStringLiteral("1.0"));
    controller.setLoaderKind(QStringLiteral("fabric"));
    QVERIFY(controller.loaderVersion().isEmpty());
}

void TstAppController::settingsRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("config.ini"));

    {
        AppSettings settings(nullptr, path);
        settings.setMinecraftDir(QStringLiteral("D:/mc-test"));
        settings.setJavaPath(QStringLiteral("D:/java/bin/java.exe"));
    }
    {
        AppSettings settings(nullptr, path);
        QCOMPARE(settings.minecraftDir(), QStringLiteral("D:/mc-test"));
        QCOMPARE(settings.javaPath(), QStringLiteral("D:/java/bin/java.exe"));
    }
}
