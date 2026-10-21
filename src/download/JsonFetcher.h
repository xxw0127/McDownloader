#ifndef JSONFETCHER_H
#define JSONFETCHER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class JsonFetcher : public QObject
{
    Q_OBJECT

public:
    explicit JsonFetcher(QObject* parent = nullptr);

    void fetch(const QList<QUrl>& urls, int timeoutMs = 15000);

signals:
    void finished(const QByteArray& body);
    void failed(const QString& error);

private:
    void startNext();
    void handleReplyFinished();
    void handleTimeout();
    void reset();

    QNetworkAccessManager* m_nam = nullptr;
    QTimer* m_timeoutTimer = nullptr;
    QNetworkReply* m_reply = nullptr;
    QList<QUrl> m_urls;
    int m_index = 0;
    int m_timeoutMs = 15000;
    QString m_lastError;
};

#endif // JSONFETCHER_H
