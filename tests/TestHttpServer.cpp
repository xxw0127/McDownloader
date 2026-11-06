#include "TestHttpServer.h"

#include <QHostAddress>
#include <QPointer>
#include <QTcpSocket>
#include <QTimer>

TestHttpServer::TestHttpServer() = default;

bool TestHttpServer::start()
{
    return listen(QHostAddress::LocalHost, 0);
}

QUrl TestHttpServer::url(const QString& path) const
{
    return QUrl(QStringLiteral("http://127.0.0.1:%1%2").arg(serverPort()).arg(path));
}

void TestHttpServer::setResponse(const QString& path, int status, const QByteArray& body, int delayMs)
{
    Response response;
    response.status = status;
    response.body = body;
    response.delayMs = delayMs;
    m_responses.insert(path, response);
}

void TestHttpServer::setSequence(const QString& path, const QList<Response>& responses)
{
    m_sequences.insert(path, responses);
}

int TestHttpServer::requestCount(const QString& path) const
{
    return m_requestCounts.value(path, 0);
}

void TestHttpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        delete socket;
        return;
    }

    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
        if (socket->property("responded").toBool())
            return;

        QByteArray buffer = socket->property("requestBuffer").toByteArray();
        buffer += socket->readAll();
        socket->setProperty("requestBuffer", buffer);

        if (!buffer.contains("\r\n\r\n"))
            return;

        const QList<QByteArray> lines = buffer.split('\n');
        if (lines.isEmpty())
        {
            socket->disconnectFromHost();
            return;
        }

        const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
        if (requestLine.size() < 2)
        {
            socket->disconnectFromHost();
            return;
        }

        QString path = QString::fromUtf8(requestLine.at(1));
        const int query = path.indexOf(QLatin1Char('?'));
        if (query >= 0)
            path = path.left(query);

        m_requestCounts[path] = m_requestCounts.value(path, 0) + 1;
        socket->setProperty("responded", true);

        Response response = m_responses.value(path, Response{404, QByteArrayLiteral("not found"), 0});
        if (m_sequences.contains(path) && !m_sequences[path].isEmpty())
            response = m_sequences[path].takeFirst();
        const QByteArray statusText = response.status == 200 ? QByteArrayLiteral("OK")
                                      : response.status == 404 ? QByteArrayLiteral("Not Found")
                                                               : QByteArrayLiteral("Error");
        QByteArray payload;
        payload += "HTTP/1.1 " + QByteArray::number(response.status) + " " + statusText + "\r\n";
        payload += "Content-Length: " + QByteArray::number(response.body.size()) + "\r\n";
        payload += "Connection: close\r\n";
        payload += "Content-Type: application/octet-stream\r\n\r\n";
        payload += response.body;

        auto sendResponse = [socket, payload]() {
            if (!socket)
                return;
            socket->write(payload);
            socket->flush();
            socket->disconnectFromHost();
        };

        if (response.delayMs > 0)
        {
            QPointer<QTcpSocket> guard(socket);
            QTimer::singleShot(response.delayMs, socket, [guard, payload]() {
                if (!guard)
                    return;
                guard->write(payload);
                guard->flush();
                guard->disconnectFromHost();
            });
        }
        else
        {
            sendResponse();
        }
    });
}
