#include "core/Logger.h"

#include <QDateTime>
#include <QDir>
#include <QMutexLocker>
#include <QTextStream>
#include <QThread>

QMutex Logger::s_mutex;
QFile Logger::s_file;
QString Logger::s_path;

void Logger::init(const QString& logDir)
{
    QMutexLocker locker(&s_mutex);

    QDir dir(logDir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral(".")))
    {
        s_path.clear();
        return;
    }

    const QString fileName = QStringLiteral("installer-%1.log").arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd")));
    const QString path = dir.filePath(fileName);

    if (s_file.isOpen())
        s_file.close();

    s_file.setFileName(path);
    if (s_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        s_path = path;
    else
        s_path.clear();
}

void Logger::shutdown()
{
    QMutexLocker locker(&s_mutex);
    if (s_file.isOpen())
        s_file.close();
    s_path.clear();
}

QString Logger::logFilePath()
{
    QMutexLocker locker(&s_mutex);
    return s_path;
}

void Logger::write(const QString& level, const QString& module, const QString& message)
{
    QMutexLocker locker(&s_mutex);

    const QString line = QStringLiteral("%1 [%2] [tid:%3] [%4] %5")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")))
                             .arg(level)
                             .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
                             .arg(module)
                             .arg(message);

    if (s_file.isOpen())
    {
        QTextStream stream(&s_file);
        stream.setCodec("UTF-8");
        stream << line << Qt::endl;
        s_file.flush();
    }
}

void Logger::debug(const QString& module, const QString& message)
{
    write(QStringLiteral("DEBUG"), module, message);
}

void Logger::info(const QString& module, const QString& message)
{
    write(QStringLiteral("INFO"), module, message);
}

void Logger::warn(const QString& module, const QString& message)
{
    write(QStringLiteral("WARN"), module, message);
}

void Logger::error(const QString& module, const QString& message)
{
    write(QStringLiteral("ERROR"), module, message);
}
