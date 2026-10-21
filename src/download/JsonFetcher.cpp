#include "download/JsonFetcher.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

JsonFetcher::JsonFetcher(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &JsonFetcher::handleTimeout);
}

void JsonFetcher::fetch(const QList<QUrl>& urls, int timeoutMs)
{
    if (m_reply != nullptr)
    {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    m_timeoutTimer->stop();

    m_urls = urls;
    m_index = 0;
    m_timeoutMs = timeoutMs;
    m_lastError.clear();

    if (m_urls.isEmpty())
    {
        emit failed(QStringLiteral("没有可用的 JSON 地址"));
        return;
    }

    startNext();
}

void JsonFetcher::startNext()
{
    if (m_index >= m_urls.size())
    {
        const QString error = m_lastError.isEmpty()
                                  ? QStringLiteral("JSON 请求失败")
                                  : m_lastError;
        reset();
        emit failed(error);
        return;
    }

    const QUrl url = m_urls.at(m_index);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PCL2Downloader/0.1"));

    m_reply = m_nam->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &JsonFetcher::handleReplyFinished);
    m_timeoutTimer->start(m_timeoutMs);
}

void JsonFetcher::handleReplyFinished()
{
    if (m_reply == nullptr)
        return;

    m_timeoutTimer->stop();

    QNetworkReply* reply = m_reply;
    m_reply = nullptr;
    disconnect(reply, nullptr, this, nullptr);

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QNetworkReply::NetworkError error = reply->error();
    const QString errorText = reply->errorString();
    reply->deleteLater();

    if (error == QNetworkReply::NoError && status >= 200 && status < 300 && !body.isEmpty())
    {
        reset();
        emit finished(body);
        return;
    }

    m_lastError = QStringLiteral("%1（HTTP %2，%3）").arg(m_urls.at(m_index).toString()).arg(status).arg(errorText);
    ++m_index;
    startNext();
}

void JsonFetcher::handleTimeout()
{
    if (m_reply == nullptr)
        return;

    QNetworkReply* reply = m_reply;
    m_reply = nullptr;
    disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    reply->deleteLater();

    m_lastError = QStringLiteral("请求超时：%1").arg(m_urls.at(m_index).toString());
    ++m_index;
    startNext();
}

void JsonFetcher::reset()
{
    m_timeoutTimer->stop();
    m_urls.clear();
    m_index = 0;
    m_lastError.clear();
}
