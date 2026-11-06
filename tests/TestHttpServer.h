#ifndef TESTHTTPSERVER_H
#define TESTHTTPSERVER_H

#include <QHash>
#include <QTcpServer>
#include <QUrl>

class TestHttpServer : public QTcpServer
{
public:
    struct Response
    {
        int status = 200;
        QByteArray body;
        int delayMs = 0;
    };

    TestHttpServer();

    bool start();
    QUrl url(const QString& path) const;
    void setResponse(const QString& path, int status, const QByteArray& body, int delayMs = 0);
    void setSequence(const QString& path, const QList<Response>& responses);
    int requestCount(const QString& path) const;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QHash<QString, Response> m_responses;
    QHash<QString, QList<Response>> m_sequences;
    mutable QHash<QString, int> m_requestCounts;
};

#endif // TESTHTTPSERVER_H
