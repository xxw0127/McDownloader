#include "tst_jsonfetcher.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "TestHttpServer.h"
#include "download/JsonFetcher.h"
#include "download/MirrorResolver.h"

void TstJsonFetcher::fallsBackToSecondJsonUrl()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/bad.json"), 500, QByteArray());
    server.setResponse(QStringLiteral("/good.json"), 200, QByteArrayLiteral("{\"ok\":true}"));

    JsonFetcher fetcher;
    QSignalSpy finished(&fetcher, &JsonFetcher::finished);
    fetcher.fetch({server.url(QStringLiteral("/bad.json")), server.url(QStringLiteral("/good.json"))});

    QVERIFY(finished.wait(5000));
    QVERIFY(finished.first().at(0).toByteArray().contains("ok"));
}

void TstJsonFetcher::timeoutEmitsFailed()
{
    TestHttpServer server;
    QVERIFY(server.start());
    server.setResponse(QStringLiteral("/slow.json"), 200, QByteArrayLiteral("{}"), 1500);

    JsonFetcher fetcher;
    QSignalSpy failed(&fetcher, &JsonFetcher::failed);
    fetcher.fetch({server.url(QStringLiteral("/slow.json"))}, 200);

    QVERIFY(failed.wait(3000));
}

void TstJsonFetcher::fetchesRealManifestWhenEnabled()
{
    if (!qEnvironmentVariableIsSet("PCL2_RUN_NETWORK_TESTS"))
        QSKIP("设置 PCL2_RUN_NETWORK_TESTS=1 才运行网络测试");

    JsonFetcher fetcher;
    QSignalSpy finished(&fetcher, &JsonFetcher::finished);
    QSignalSpy failed(&fetcher, &JsonFetcher::failed);
    fetcher.fetch(MirrorResolver::manifestUrls(), 20000);

    if (finished.isEmpty())
        QVERIFY(finished.wait(25000));

    const QByteArray body = finished.first().at(0).toByteArray();
    const QJsonDocument document = QJsonDocument::fromJson(body);
    QVERIFY(document.isObject());
    QVERIFY(document.object().value(QStringLiteral("versions")).isArray());
    QVERIFY(failed.isEmpty());
}
