#include "tst_download.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "TestHttpServer.h"
#include "download/DownloadService.h"

namespace
{
QByteArray sha1(const QByteArray& data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Sha1);
}

FileSpec makeSpec(const QString& target, const QList<QUrl>& urls, const QByteArray& body)
{
    FileSpec spec;
    spec.targetRelativePath = target;
    spec.urls = urls;
    spec.size = body.size();
    spec.sha1 = sha1(body);
    spec.required = true;
    return spec;
}
}

void TstDownload::downloadsAndVerifies()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/a.bin"), 200, QByteArrayLiteral("abc"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DownloadService service;
    QSignalSpy finished(&service, &DownloadService::finished);

    const FileSpec spec = makeSpec(QStringLiteral("a.bin"), {server.url(QStringLiteral("/a.bin"))}, QByteArrayLiteral("abc"));
    service.start(dir.path(), {spec});

    QVERIFY(finished.wait(5000));
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/a.bin")));
    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/a.bin.part")));
}

void TstDownload::fallsBackToSecondUrl()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/bad.bin"), 500, QByteArray());
    server.setResponse(QStringLiteral("/good.bin"), 200, QByteArrayLiteral("ok"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DownloadService service;
    QSignalSpy finished(&service, &DownloadService::finished);

    const FileSpec spec = makeSpec(QStringLiteral("good.bin"),
                                   {server.url(QStringLiteral("/bad.bin")), server.url(QStringLiteral("/good.bin"))},
                                   QByteArrayLiteral("ok"));
    service.start(dir.path(), {spec});

    QVERIFY(finished.wait(5000));
    QVERIFY(QFile::exists(dir.path() + QStringLiteral("/good.bin")));
    QCOMPARE(server.requestCount(QStringLiteral("/bad.bin")), 1);
    QCOMPARE(server.requestCount(QStringLiteral("/good.bin")), 1);
}

void TstDownload::rejectsBadSha1()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/bad.bin"), 200, QByteArrayLiteral("bad"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DownloadService service;
    QSignalSpy failed(&service, &DownloadService::failed);

    FileSpec spec = makeSpec(QStringLiteral("bad.bin"), {server.url(QStringLiteral("/bad.bin"))}, QByteArrayLiteral("good"));
    service.start(dir.path(), {spec});

    QVERIFY(failed.wait(5000));
    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/bad.bin")));
}

void TstDownload::skipsExistingValidFile()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/a.bin"), 200, QByteArrayLiteral("abc"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile file(dir.path() + QStringLiteral("/a.bin"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("abc");
    file.close();

    DownloadService service;
    QSignalSpy finished(&service, &DownloadService::finished);

    const FileSpec spec = makeSpec(QStringLiteral("a.bin"), {server.url(QStringLiteral("/a.bin"))}, QByteArrayLiteral("abc"));
    service.start(dir.path(), {spec});

    // 全部文件已存在时 finished 可以同步发出，wait 只用于异步路径
    if (finished.isEmpty())
        QVERIFY(finished.wait(5000));
    QCOMPARE(server.requestCount(QStringLiteral("/a.bin")), 0);
}

void TstDownload::cancelLeavesNoFinalFile()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/slow.bin"), 200, QByteArrayLiteral("slow"), 1500);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DownloadService service;
    QSignalSpy canceled(&service, &DownloadService::canceled);

    FileSpec spec = makeSpec(QStringLiteral("slow.bin"), {server.url(QStringLiteral("/slow.bin"))}, QByteArrayLiteral("slow"));
    service.start(dir.path(), {spec});

    QTest::qWait(150);
    service.cancel();
    if (canceled.isEmpty())
        QVERIFY(canceled.wait(3000));

    QVERIFY(!QFile::exists(dir.path() + QStringLiteral("/slow.bin")));
}

void TstDownload::rejectsUnsafeRelativePath()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/a.bin"), 200, QByteArrayLiteral("abc"));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DownloadService service;
    QSignalSpy failed(&service, &DownloadService::failed);

    FileSpec spec = makeSpec(QStringLiteral("../escape-mcdownloader.bin"),
                             {server.url(QStringLiteral("/a.bin"))}, QByteArrayLiteral("abc"));
    service.start(dir.path(), {spec});

    if (failed.isEmpty())
        QVERIFY(failed.wait(3000));

    QVERIFY(!QFile::exists(QDir(dir.path()).filePath(QStringLiteral("../escape-mcdownloader.bin"))));
}
