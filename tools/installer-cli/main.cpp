#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>

#include "core/Logger.h"
#include "installers/InstallController.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("installer-cli"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Headless vanilla Minecraft installer"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption mcOption(QStringLiteral("mc"), QStringLiteral("Minecraft version id"), QStringLiteral("id"));
    QCommandLineOption dirOption(QStringLiteral("dir"), QStringLiteral("Target .minecraft folder"), QStringLiteral("path"));
    QCommandLineOption javaOption(QStringLiteral("java"), QStringLiteral("java.exe path"), QStringLiteral("path"));
    parser.addOption(mcOption);
    parser.addOption(dirOption);
    parser.addOption(javaOption);
    parser.process(app);

    if (!parser.isSet(mcOption) || !parser.isSet(dirOption))
    {
        parser.showHelp(2);
        return 2;
    }

    InstallRequest request;
    request.mcVersion = parser.value(mcOption);
    request.minecraftDir = parser.value(dirOption);
    request.javaPath = parser.value(javaOption);

    Logger::init(QDir::current().filePath(QStringLiteral("logs")));
    Logger::info(QStringLiteral("cli"),
                 QStringLiteral("install mc=%1 dir=%2").arg(request.mcVersion, request.minecraftDir));

    InstallController controller;
    QTextStream out(stdout);
    QTextStream err(stderr);
    out.setCodec("UTF-8");
    err.setCodec("UTF-8");

    int lastPercent = -1;
    QObject::connect(&controller, &InstallController::progress, [&](qint64 received, qint64 total, const QString& status) {
        if (total > 0)
        {
            const int percent = qBound(0, int((received * 100) / total), 100);
            if (percent != lastPercent)
            {
                lastPercent = percent;
                out << "progress " << percent << "% " << status << Qt::endl;
            }
        }
        else
        {
            out << "progress " << status << Qt::endl;
        }
    });

    int exitCode = -1;
    QObject::connect(&controller, &InstallController::finished, [&](const QString& versionId) {
        out << "installed " << versionId << Qt::endl;
        exitCode = 0;
        app.exit(0);
    });

    QObject::connect(&controller, &InstallController::failed, [&](const QString& error) {
        err << "error: " << error << Qt::endl;
        Logger::error(QStringLiteral("cli"), error);
        exitCode = 1;
        app.exit(1);
    });

    QObject::connect(&controller, &InstallController::canceled, [&]() {
        out << "canceled" << Qt::endl;
        exitCode = 2;
        app.exit(2);
    });

    controller.start(request);
    if (exitCode >= 0)
        return exitCode;
    return app.exec();
}
