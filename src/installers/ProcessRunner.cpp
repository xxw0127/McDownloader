#include "installers/ProcessRunner.h"

#include <QElapsedTimer>
#include <QProcess>

ProcessResult ProcessRunner::run(const QString& program,
                                 const QStringList& args,
                                 const QProcessEnvironment& env,
                                 int timeoutMs,
                                 const std::atomic_bool* cancelFlag)
{
    ProcessResult result;
    QProcess process;
    if (!env.isEmpty())
        process.setProcessEnvironment(env);

    process.setProgram(program);
    process.setArguments(args);
    process.start();

    if (!process.waitForStarted(5000))
    {
        result.stdErr = process.errorString();
        return result;
    }

    QElapsedTimer timer;
    timer.start();
    bool finished = false;

    while (timer.elapsed() < timeoutMs)
    {
        if (process.waitForFinished(200))
        {
            finished = true;
            break;
        }

        if (cancelFlag != nullptr && cancelFlag->load())
        {
            result.canceled = true;
            process.kill();
            process.waitForFinished(2000);
            break;
        }
    }

    if (!finished && !result.canceled)
    {
        result.timedOut = true;
        process.kill();
        process.waitForFinished(2000);
    }

    result.exitCode = process.exitCode();
    result.stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    result.stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    if (result.stdErr.isEmpty() && process.exitStatus() == QProcess::CrashExit &&
        !result.timedOut && !result.canceled)
        result.stdErr = process.errorString();
    return result;
}
