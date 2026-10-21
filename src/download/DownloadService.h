#ifndef DOWNLOADSERVICE_H
#define DOWNLOADSERVICE_H

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVector>

#include "download/FileSpec.h"

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class DownloadService : public QObject
{
    Q_OBJECT

public:
    explicit DownloadService(QObject* parent = nullptr);

    void start(const QString& rootDir, const QVector<FileSpec>& files, int concurrency = 8);
    void cancel();
    bool isRunning() const;

signals:
    void progress(qint64 received, qint64 total, const QString& currentFile);
    void fileFinished(const QString& path);
    void finished();
    void failed(const QString& error);
    void canceled();

private:
    struct Task;

    void pump();
    void startTask(Task* task);
    void startUrl(Task* task);
    void handleReplyFinished(Task* task);
    void failAttempt(Task* task, const QString& error);
    void failDownload(const QString& error);
    void cleanupTask(Task* task);
    void abortAll();
    void emitProgress(const QString& currentFile, bool force = false);
    QString targetPathFor(const FileSpec& spec, bool* ok) const;

    QNetworkAccessManager* m_nam = nullptr;
    QString m_root;
    QVector<FileSpec> m_pending;
    QVector<Task*> m_active;
    int m_concurrency = 8;
    qint64 m_total = 0;
    qint64 m_received = 0;
    qint64 m_lastProgressMs = 0;
    bool m_running = false;
    bool m_canceled = false;
    QElapsedTimer m_progressTimer;
};

#endif // DOWNLOADSERVICE_H
