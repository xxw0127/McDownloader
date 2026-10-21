#include <QGuiApplication>
#include <QMessageLogContext>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "AppController.h"
#include "core/Logger.h"
#include "platform/PlatformPaths.h"

namespace
{
void logQtMessage(QtMsgType type, const QMessageLogContext&, const QString& message)
{
    switch (type)
    {
    case QtDebugMsg:
        Logger::debug(QStringLiteral("Qt"), message);
        break;
    case QtInfoMsg:
        Logger::info(QStringLiteral("Qt"), message);
        break;
    case QtWarningMsg:
        Logger::warn(QStringLiteral("Qt"), message);
        break;
    case QtCriticalMsg:
        Logger::error(QStringLiteral("Qt"), message);
        break;
    case QtFatalMsg:
        Logger::error(QStringLiteral("Qt"), message);
        abort();
    }
}
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("PCL2Downloader"));
    QCoreApplication::setApplicationName(QStringLiteral("PCL2 Downloader"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    Logger::init(PlatformPaths::logDir());
    qInstallMessageHandler(logQtMessage);
    Logger::info(QStringLiteral("main"), QStringLiteral("启动 PCL2 下载器"));

    AppController controller;
    controller.loadMockVersions();

    if (QCoreApplication::arguments().contains(QStringLiteral("--ui-smoke")))
    {
        QMetaObject::invokeMethod(controller.versionModel(), "toggleGroup", Q_ARG(int, 0));
        controller.setSelectedVersion(QStringLiteral("26.3"));
    }

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
