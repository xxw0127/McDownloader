#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QString>

class Logger
{
public:
    static void init(const QString& logDir);
    static void shutdown();
    static void debug(const QString& module, const QString& message);
    static void info(const QString& module, const QString& message);
    static void warn(const QString& module, const QString& message);
    static void error(const QString& module, const QString& message);
    static QString logFilePath();

private:
    static void write(const QString& level, const QString& module, const QString& message);

    static QMutex s_mutex;
    static QFile s_file;
    static QString s_path;
};

#endif // LOGGER_H
