#include "download/DownloadService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "core/Checksum.h"
#include "core/Logger.h"
#include "core/Paths.h"

struct DownloadService::Task
{
    FileSpec spec;
    int urlIndex = 0;
    QNetworkReply* reply = nullptr;
    QFile* file = nullptr;
    qint64 received = 0;
};

DownloadService::DownloadService(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

bool DownloadService::isRunning() const
{
    return m_running;
}

void DownloadService::start(const QString& rootDir, const QVector<FileSpec>& files, int concurrency)
{
    if (m_running)
    {
        emit failed(QStringLiteral("下载已在进行"));
        return;
    }

    m_root = rootDir;
    m_pending.clear();
    m_active.clear();
    m_concurrency = qMax(1, concurrency);
    m_total = 0;
    m_received = 0;
    m_lastProgressMs = 0;
    m_running = true;
    m_canceled = false;
    m_progressTimer.start();

    if (!QDir().mkpath(m_root))
    {
        failDownload(QStringLiteral("无法创建下载目录：%1").arg(m_root));
        return;
    }

    for (const FileSpec& spec : files)
    {
        bool ok = false;
        const QString target = targetPathFor(spec, &ok);
        if (!ok || target.isEmpty())
        {
            failDownload(QStringLiteral("不安全的下载路径：%1").arg(spec.targetRelativePath));
            return;
        }
    }

    for (const FileSpec& spec : files)
    {
        bool ok = false;
        const QString target = targetPathFor(spec, &ok);
        if (!ok)
        {
            failDownload(QStringLiteral("不安全的下载路径：%1").arg(spec.targetRelativePath));
            return;
        }

        if (QFileInfo::exists(target))
        {
            QString error;
            if (Checksum::verifyFile(target, spec.size, spec.sha1, &error))
            {
                emit fileFinished(target);
                continue;
            }
        }

        m_pending.append(spec);
        if (spec.size > 0)
            m_total += spec.size;
    }

    if (m_pending.isEmpty())
    {
        m_running = false;
        emit finished();
        return;
    }

    pump();
}

void DownloadService::cancel()
{
    if (!m_running)
        return;

    m_canceled = true;
    m_running = false;
    abortAll();
    emit canceled();
}

void DownloadService::pump()
{
    while (m_running && !m_canceled && m_active.size() < m_concurrency && !m_pending.isEmpty())
    {
        Task* task = new Task;
        task->spec = m_pending.takeFirst();
        m_active.append(task);
        startTask(task);
    }

    if (m_running && !m_canceled && m_active.isEmpty() && m_pending.isEmpty())
    {
        m_running = false;
        emit finished();
    }
}

void DownloadService::startTask(Task* task)
{
    bool ok = false;
    const QString target = targetPathFor(task->spec, &ok);
    if (!ok || target.isEmpty())
    {
        failDownload(QStringLiteral("不安全的下载路径：%1").arg(task->spec.targetRelativePath));
        return;
    }

    if (!QDir().mkpath(QFileInfo(target).absolutePath()))
    {
        failDownload(QStringLiteral("无法创建目录：%1").arg(QFileInfo(target).absolutePath()));
        return;
    }

    if (task->file != nullptr)
    {
        task->file->close();
        delete task->file;
        task->file = nullptr;
    }

    task->file = new QFile(target + QStringLiteral(".part"));
    if (!task->file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        failDownload(QStringLiteral("无法写入文件：%1").arg(task->file->fileName()));
        return;
    }

    task->received = 0;
    startUrl(task);
}

void DownloadService::startUrl(Task* task)
{
    if (task->urlIndex >= task->spec.urls.size())
    {
        failAttempt(task, QStringLiteral("所有下载地址均失败：%1").arg(task->spec.targetRelativePath));
        return;
    }

    const QUrl url = task->spec.urls.at(task->urlIndex);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PCL2Downloader/0.1"));

    task->reply = m_nam->get(request);

    connect(task->reply, &QNetworkReply::readyRead, this, [this, task]() {
        if (task->reply == nullptr || task->file == nullptr)
            return;

        const QByteArray data = task->reply->readAll();
        if (data.isEmpty())
            return;

        const qint64 written = task->file->write(data);
        if (written != data.size())
        {
            task->reply->abort();
            return;
        }

        task->received += data.size();
        m_received += data.size();
        emitProgress(task->reply->url().toString());
    });

    connect(task->reply, &QNetworkReply::finished, this, [this, task]() {
        handleReplyFinished(task);
    });
}

void DownloadService::handleReplyFinished(Task* task)
{
    if (task->reply == nullptr)
        return;

    QNetworkReply* reply = task->reply;
    task->reply = nullptr;
    disconnect(reply, nullptr, this, nullptr);

    const QByteArray remaining = reply->readAll();
    if (!remaining.isEmpty() && task->file != nullptr)
    {
        task->file->write(remaining);
        task->received += remaining.size();
        m_received += remaining.size();
    }

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError error = reply->error();
    const QString errorText = reply->errorString();
    reply->deleteLater();

    if (m_canceled || !m_running)
    {
        cleanupTask(task);
        return;
    }

    if (error != QNetworkReply::NoError || status < 200 || status >= 300)
    {
        Logger::warn(QStringLiteral("download"),
                     QStringLiteral("URL 失败：%1（HTTP %2，%3）")
                         .arg(task->spec.urls.at(task->urlIndex).toString())
                         .arg(status)
                         .arg(errorText));
        failAttempt(task, QStringLiteral("下载失败：%1（HTTP %2，%3）")
                              .arg(task->spec.urls.at(task->urlIndex).toString())
                              .arg(status)
                              .arg(errorText));
        return;
    }

    if (task->file != nullptr)
    {
        task->file->flush();
        task->file->close();
        delete task->file;
        task->file = nullptr;
    }

    bool ok = false;
    const QString target = targetPathFor(task->spec, &ok);
    if (!ok)
    {
        failAttempt(task, QStringLiteral("不安全的下载路径：%1").arg(task->spec.targetRelativePath));
        return;
    }

    const QString partPath = target + QStringLiteral(".part");
    QString verifyError;
    if (!Checksum::verifyFile(partPath, task->spec.size, task->spec.sha1, &verifyError))
    {
        QFile::remove(partPath);
        failAttempt(task, verifyError);
        return;
    }

    QFile::remove(target);
    if (!QFile::rename(partPath, target))
    {
        QFile::remove(partPath);
        failAttempt(task, QStringLiteral("无法重命名下载文件：%1").arg(target));
        return;
    }

    emit fileFinished(target);
    cleanupTask(task);
    pump();
}

void DownloadService::failAttempt(Task* task, const QString& error)
{
    m_received -= task->received;
    if (m_received < 0)
        m_received = 0;
    task->received = 0;

    if (task->reply != nullptr)
    {
        QNetworkReply* reply = task->reply;
        task->reply = nullptr;
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }

    if (task->file != nullptr)
    {
        task->file->close();
        delete task->file;
        task->file = nullptr;
    }

    bool ok = false;
    const QString target = targetPathFor(task->spec, &ok);
    if (ok)
        QFile::remove(target + QStringLiteral(".part"));

    ++task->urlIndex;
    if (task->urlIndex < task->spec.urls.size())
    {
        startTask(task);
        return;
    }

    if (task->spec.required)
    {
        m_running = false;
        abortAll();
        emit failed(error);
        return;
    }

    cleanupTask(task);
    pump();
}

void DownloadService::failDownload(const QString& error)
{
    m_running = false;
    abortAll();
    emit failed(error);
}

void DownloadService::cleanupTask(Task* task)
{
    if (task == nullptr)
        return;

    if (task->reply != nullptr)
    {
        QNetworkReply* reply = task->reply;
        task->reply = nullptr;
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }

    if (task->file != nullptr)
    {
        task->file->close();
        delete task->file;
        task->file = nullptr;
    }

    m_active.removeAll(task);
    delete task;
}

void DownloadService::abortAll()
{
    const QVector<Task*> active = m_active;
    for (Task* task : active)
        cleanupTask(task);
    m_active.clear();
    m_pending.clear();
}

void DownloadService::emitProgress(const QString& currentFile, bool force)
{
    const qint64 elapsed = m_progressTimer.isValid() ? m_progressTimer.elapsed() : 0;
    if (!force && elapsed - m_lastProgressMs < 100)
        return;

    m_lastProgressMs = elapsed;
    emit progress(m_received, m_total, currentFile);
}

QString DownloadService::targetPathFor(const FileSpec& spec, bool* ok) const
{
    return Paths::joinWithin(m_root, spec.targetRelativePath, ok);
}
