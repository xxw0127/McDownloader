#include "tst_process.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QTemporaryDir>

#include <atomic>
#include <chrono>
#include <thread>

#include "installers/ProcessRunner.h"
#include "platform/PlatformProcess.h"

void TstProcess::capturesExitCode()
{
    const ProcessResult result = ProcessRunner::run(
        PlatformProcess::cmdExecutable(),
        {QStringLiteral("/c"), QStringLiteral("exit"), QStringLiteral("3")},
        QProcessEnvironment(),
        5000);
    QCOMPARE(result.exitCode, 3);
    QVERIFY(!result.timedOut);
}

void TstProcess::capturesStdout()
{
    const ProcessResult result = ProcessRunner::run(
        PlatformProcess::cmdExecutable(),
        {QStringLiteral("/c"), QStringLiteral("echo"), QStringLiteral("hello")},
        QProcessEnvironment(),
        5000);
    QCOMPARE(result.exitCode, 0);
    QCOMPARE(result.stdOut.trimmed(), QStringLiteral("hello"));
}

void TstProcess::timesOut()
{
    const ProcessResult result = ProcessRunner::run(
        PlatformProcess::cmdExecutable(),
        {QStringLiteral("/c"), QStringLiteral("ping"), QStringLiteral("127.0.0.1"),
         QStringLiteral("-n"), QStringLiteral("5"), QStringLiteral(">"), QStringLiteral("nul")},
        QProcessEnvironment(),
        200);
    QVERIFY(result.timedOut);
}

void TstProcess::reportsMissingExecutable()
{
    const ProcessResult result = ProcessRunner::run(
        QStringLiteral("Z:/definitely/not/here.exe"),
        {},
        QProcessEnvironment(),
        1000);
    QCOMPARE(result.exitCode, -1);
    QVERIFY(!result.stdErr.isEmpty());
}

void TstProcess::createsDirectoryLink()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString target = dir.filePath(QStringLiteral("target"));
    const QString link = dir.filePath(QStringLiteral("link"));
    QVERIFY(QDir().mkpath(target));
    QFile marker(QDir(target).filePath(QStringLiteral("marker.txt")));
    QVERIFY(marker.open(QIODevice::WriteOnly));
    marker.write("ok");
    marker.close();

    QString error;
    QVERIFY2(PlatformProcess::createDirectoryLink(link, target, &error), qPrintable(error));
    QVERIFY(QFileInfo(link).isDir());
    QVERIFY(QFileInfo(QDir(link).filePath(QStringLiteral("marker.txt"))).exists());
}

void TstProcess::cancelStopsRunningProcess()
{
    std::atomic_bool cancel{false};
    std::thread canceler([&cancel]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        cancel.store(true);
    });

    const ProcessResult result = ProcessRunner::run(
        PlatformProcess::cmdExecutable(),
        {QStringLiteral("/c"), QStringLiteral("ping"), QStringLiteral("127.0.0.1"),
         QStringLiteral("-n"), QStringLiteral("10"), QStringLiteral(">"), QStringLiteral("nul")},
        QProcessEnvironment(),
        10000,
        &cancel);

    canceler.join();
    QVERIFY(result.canceled);
    QVERIFY(!result.timedOut);
}
