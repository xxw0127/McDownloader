#include <QCoreApplication>
#include <QtTest>

#include "tst_checksum.h"
#include "tst_logger.h"
#include "tst_paths.h"
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
    return status;
}
