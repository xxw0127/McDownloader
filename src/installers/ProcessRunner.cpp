#include "installers/ProcessRunner.h"

#include <QProcess>

ProcessResult ProcessRunner::run(const QString& program,
                                 const QStringList& args,
                                 const QProcessEnvironment& env,
                                 int timeoutMs)
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

    if (!process.waitForFinished(timeoutMs))
    {
        result.timedOut = true;
        process.kill();
        process.waitForFinished(2000);
    }

    result.exitCode = process.exitCode();
    result.stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    result.stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    if (result.stdErr.isEmpty() && process.exitStatus() == QProcess::CrashExit && !result.timedOut)
        result.stdErr = process.errorString();
    return result;
}
