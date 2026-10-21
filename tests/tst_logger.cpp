#include "tst_logger.h"

#include "core/Logger.h"

void TstLogger::writesInfoLine()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Logger::init(dir.path());
    Logger::info(QStringLiteral("test"), QStringLiteral("hello"));

    const QStringList files = QDir(dir.path()).entryList(QStringList() << QStringLiteral("*.log"), QDir::Files);
    QCOMPARE(files.size(), 1);

    QFile file(dir.path() + QLatin1Char('/') + files.first());
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray content = file.readAll();
    QVERIFY(content.contains("[INFO]"));
    QVERIFY(content.contains("hello"));

    Logger::shutdown();
    QVERIFY(Logger::logFilePath().isEmpty());
}

void TstLogger::writesErrorLine()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Logger::init(dir.path());
    Logger::error(QStringLiteral("test"), QStringLiteral("boom"));

    const QStringList files = QDir(dir.path()).entryList(QStringList() << QStringLiteral("*.log"), QDir::Files);
    QCOMPARE(files.size(), 1);

    QFile file(dir.path() + QLatin1Char('/') + files.first());
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(file.readAll().contains("[ERROR]"));

    Logger::shutdown();
    QVERIFY(Logger::logFilePath().isEmpty());
}

void TstLogger::writesUtf8Chinese()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Logger::init(dir.path());
    Logger::info(QStringLiteral("test"), QStringLiteral("中文日志"));

    const QStringList files = QDir(dir.path()).entryList(QStringList() << QStringLiteral("*.log"), QDir::Files);
    QCOMPARE(files.size(), 1);

    QFile file(dir.path() + QLatin1Char('/') + files.first());
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray content = file.readAll();
    QVERIFY(content.contains(QStringLiteral("中文日志").toUtf8()));

    Logger::shutdown();
    QVERIFY(Logger::logFilePath().isEmpty());
}
