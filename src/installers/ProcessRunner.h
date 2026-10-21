#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

#include <atomic>

struct ProcessResult
{
    int exitCode = -1;
    QString stdOut;
    QString stdErr;
    bool timedOut = false;
    bool canceled = false;
};

class ProcessRunner
{
public:
    static ProcessResult run(const QString& program,
                             const QStringList& args,
                             const QProcessEnvironment& env,
                             int timeoutMs,
                             const std::atomic_bool* cancelFlag = nullptr);
};

#endif // PROCESSRUNNER_H
