#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QImage>
#include <QMessageLogContext>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("McDownloader"));
    QCoreApplication::setApplicationName(QStringLiteral("McDownloader"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QFont appFont(QStringLiteral("Microsoft YaHei UI"));
    appFont.setPixelSize(13);
    app.setFont(appFont);
    app.setWindowIcon(QIcon(QStringLiteral(":/images/app_icon.png")));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    Logger::init(PlatformPaths::logDir());
    qInstallMessageHandler(logQtMessage);
    Logger::info(QStringLiteral("main"), QStringLiteral("启动 McDownloader"));

    const QStringList arguments = QCoreApplication::arguments();

    AppController controller;
    if (arguments.contains(QStringLiteral("--ui-smoke")))
    {
        controller.loadMockVersions();
        controller.setSelectedVersion(QStringLiteral("26.3"));
    }
    else
    {
        controller.refreshVersions();
    }

    const int loaderIndex = arguments.indexOf(QStringLiteral("--loader"));
    if (loaderIndex >= 0 && loaderIndex + 1 < arguments.size())
        controller.setLoaderKind(arguments.at(loaderIndex + 1));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
    {
        Logger::error(QStringLiteral("main"), QStringLiteral("QML 加载失败"));
        return 1;
    }

    // Debug aid used for visual checks: render the window to a PNG and exit.
    // Unlike a screen capture this is unaffected by other windows on the display.
    const int shotIndex = arguments.indexOf(QStringLiteral("--screenshot"));
    if (shotIndex >= 0 && shotIndex + 1 < arguments.size())
    {
        const QString path = arguments.at(shotIndex + 1);
        QTimer::singleShot(6500, &app, [&app, &engine, path]() {
            if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first()))
            {
                const QImage image = window->grabWindow();
                if (!image.isNull() && image.save(path))
                    Logger::info(QStringLiteral("main"), QStringLiteral("已保存界面截图：%1").arg(path));
                else
                    Logger::warn(QStringLiteral("main"), QStringLiteral("界面截图保存失败"));
            }
            app.quit();
        });
    }

    const int result = app.exec();
    Logger::info(QStringLiteral("main"), QStringLiteral("退出，代码 %1").arg(result));
    return result;
}
