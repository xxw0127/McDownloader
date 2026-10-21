#include <QCoreApplication>
#include <QtTest>

#include "tst_checksum.h"
#include "tst_download.h"
#include "tst_installcontroller.h"
#include "tst_jsonfetcher.h"
#include "tst_logger.h"
#include "tst_mirror.h"
#include "tst_paths.h"
#include "tst_vanilla.h"
#include "tst_version.h"
#include "tst_versionlistmodel.h"

namespace
{
int runSuite(QObject* suite, const QString& name, int argc, char *argv[])
{
    QList<QByteArray> args;
    args.reserve(argc + 2);
    for (int i = 0; i < argc; ++i)
        args.append(QByteArray(argv[i]));

    const QByteArray outputFile = (name + QStringLiteral(".txt")).toUtf8();
    args.append(QByteArrayLiteral("-o"));
    args.append(outputFile + QByteArrayLiteral(",txt"));

    QVector<char *> rawArgs;
    rawArgs.reserve(args.size());
    for (QByteArray& arg : args)
        rawArgs.append(arg.data());

    return QTest::qExec(suite, rawArgs.size(), rawArgs.data());
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int status = 0;
    {
        TstPaths test;
        status |= runSuite(&test, QStringLiteral("paths"), argc, argv);
    }
    {
        TstChecksum test;
        status |= runSuite(&test, QStringLiteral("checksum"), argc, argv);
    }
    {
        TstLogger test;
        status |= runSuite(&test, QStringLiteral("logger"), argc, argv);
    }
    {
        TstVersion test;
        status |= runSuite(&test, QStringLiteral("version"), argc, argv);
    }
    {
        TstVersionListModel test;
        status |= runSuite(&test, QStringLiteral("versionlistmodel"), argc, argv);
    }
    {
        TstDownload test;
        status |= runSuite(&test, QStringLiteral("download"), argc, argv);
    }
    {
        TstJsonFetcher test;
        status |= runSuite(&test, QStringLiteral("jsonfetcher"), argc, argv);
    }
    {
        TstMirror test;
        status |= runSuite(&test, QStringLiteral("mirror"), argc, argv);
    }
    {
        TstVanilla test;
        status |= runSuite(&test, QStringLiteral("vanilla"), argc, argv);
    }
    {
        TstInstallController test;
        status |= runSuite(&test, QStringLiteral("installcontroller"), argc, argv);
    }
    return status;
}
