#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "AppController.h"
#include "core/Logger.h"
#include "platform/PlatformPaths.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("PCL2Downloader"));
    QCoreApplication::setApplicationName(QStringLiteral("PCL2 Downloader"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    Logger::init(PlatformPaths::logDir());
    Logger::info(QStringLiteral("main"), QStringLiteral("启动 PCL2 下载器"));

    AppController controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
    {
        Logger::error(QStringLiteral("main"), QStringLiteral("QML 加载失败"));
        return 1;
    }

    const int result = app.exec();
    Logger::info(QStringLiteral("main"), QStringLiteral("退出，代码 %1").arg(result));
    return result;
}
