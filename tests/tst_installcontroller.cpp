#include "tst_installcontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "installers/InstallController.h"
#include "installers/VanillaInstallValidator.h"

namespace
{
void writeJson(const QString& path, const QJsonObject& object)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
}

void writeFile(const QString& path, const QByteArray& data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}
}

void TstInstallController::missingVersionJsonFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(!VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1")).isEmpty());
}

void TstInstallController::invalidVersionJsonFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    writeFile(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json")),
              QByteArrayLiteral("not json"));
    QVERIFY(!VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1")).isEmpty());
}

void TstInstallController::missingClientJarFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject version;
    version.insert(QStringLiteral("downloads"), QJsonObject{
        {QStringLiteral("client"), QJsonObject{{QStringLiteral("size"), 1}}}
    });
    writeJson(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json")), version);
    QVERIFY(!VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1")).isEmpty());
}

void TstInstallController::missingAssetIndexFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject version;
    version.insert(QStringLiteral("downloads"), QJsonObject{
        {QStringLiteral("client"), QJsonObject{{QStringLiteral("size"), 1}}}
    });
    version.insert(QStringLiteral("assetIndex"), QJsonObject{{QStringLiteral("id"), QStringLiteral("5")}});
    writeJson(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json")), version);
    writeFile(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.jar")), QByteArrayLiteral("jar"));
    QVERIFY(!VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1")).isEmpty());
}

void TstInstallController::missingLibraryFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject version;
    version.insert(QStringLiteral("downloads"), QJsonObject{
        {QStringLiteral("client"), QJsonObject{{QStringLiteral("size"), 1}}}
    });
    version.insert(QStringLiteral("libraries"), QJsonArray{
        QJsonObject{{QStringLiteral("name"), QStringLiteral("org.example:demo:1.0")}}
    });
    writeJson(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json")), version);
    writeFile(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.jar")), QByteArrayLiteral("jar"));

    const QString error = VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1"));
    QVERIFY(error.contains(QStringLiteral("库文件")));
}

void TstInstallController::incompleteBaseIsNotOverwritten()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString versionPath = QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json"));
    QJsonObject version;
    version.insert(QStringLiteral("downloads"), QJsonObject{
        {QStringLiteral("client"), QJsonObject{{QStringLiteral("size"), 1}}}
    });
    version.insert(QStringLiteral("libraries"), QJsonArray{
        QJsonObject{{QStringLiteral("name"), QStringLiteral("org.example:demo:1.0")}}
    });
    writeJson(versionPath, version);
    writeFile(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.jar")), QByteArrayLiteral("jar"));
    QFile before(versionPath);
    QVERIFY(before.open(QIODevice::ReadOnly));
    const QByteArray original = before.readAll();
    before.close();

    InstallController controller;
    QSignalSpy failed(&controller, &InstallController::failed);
    InstallRequest request;
    request.minecraftDir = dir.path();
    request.mcVersion = QStringLiteral("1.20.1");
    request.loader = LoaderKind::Fabric;
    request.loaderVersion = QStringLiteral("0.15.11");
    controller.start(request);

    QCOMPARE(failed.count(), 1);
    QFile after(versionPath);
    QVERIFY(after.open(QIODevice::ReadOnly));
    QCOMPARE(after.readAll(), original);
}

void TstInstallController::completeInstallPasses()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QJsonObject version;
    version.insert(QStringLiteral("downloads"), QJsonObject{
        {QStringLiteral("client"), QJsonObject{{QStringLiteral("size"), 1}}}
    });
    version.insert(QStringLiteral("assetIndex"), QJsonObject{{QStringLiteral("id"), QStringLiteral("5")}});
    version.insert(QStringLiteral("libraries"), QJsonArray{
        QJsonObject{{QStringLiteral("name"), QStringLiteral("org.example:demo:1.0")}}
    });
    writeJson(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.json")), version);
    writeFile(QDir(dir.path()).filePath(
                  QStringLiteral("libraries/org/example/demo/1.0/demo-1.0.jar")),
              QByteArrayLiteral("jar"));
    writeFile(QDir(dir.path()).filePath(QStringLiteral("versions/1.20.1/1.20.1.jar")), QByteArrayLiteral("jar"));
    writeFile(QDir(dir.path()).filePath(QStringLiteral("assets/indexes/5.json")), QByteArrayLiteral("{}"));

    QVERIFY(VanillaInstallValidator::validate(dir.path(), QStringLiteral("1.20.1")).isEmpty());
}

void TstInstallController::rejectsUnsupportedLoader()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    InstallController controller;
    QSignalSpy failed(&controller, &InstallController::failed);

    InstallRequest request;
    request.minecraftDir = dir.path();
    request.mcVersion = QStringLiteral("1.20.1");
    request.loader = static_cast<LoaderKind>(99);
    controller.start(request);

    QCOMPARE(failed.count(), 1);
    QVERIFY(!controller.isRunning());
}

void TstInstallController::rejectsUnsafeVersionId()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    InstallController controller;
    QSignalSpy failed(&controller, &InstallController::failed);
    InstallRequest request;
    request.minecraftDir = dir.path();
    request.mcVersion = QStringLiteral("../escape");
    controller.start(request);

    QCOMPARE(failed.count(), 1);
    QVERIFY(!QFileInfo::exists(QDir(dir.path()).filePath(QStringLiteral("../escape"))));

    InstallController loaderController;
    QSignalSpy loaderFailed(&loaderController, &InstallController::failed);
    InstallRequest loaderRequest;
    loaderRequest.minecraftDir = dir.path();
    loaderRequest.mcVersion = QStringLiteral("1.20.1");
    loaderRequest.loader = LoaderKind::Fabric;
    loaderRequest.loaderVersion = QStringLiteral("../escape");
    loaderController.start(loaderRequest);
    QCOMPARE(loaderFailed.count(), 1);
}

void TstInstallController::startAndCancelAreInvokable()
{
    InstallController controller;
    QVERIFY(controller.metaObject()->indexOfMethod("start(InstallRequest)") >= 0);
    QVERIFY(controller.metaObject()->indexOfMethod("cancel()") >= 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    InstallRequest request;
    request.minecraftDir = dir.path();
    request.mcVersion = QString();

    QSignalSpy failed(&controller, &InstallController::failed);
    QVERIFY(QMetaObject::invokeMethod(&controller, "start", Qt::DirectConnection,
                                      Q_ARG(InstallRequest, request)));
    QCOMPARE(failed.count(), 1);
}
