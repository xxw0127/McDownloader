#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

struct ProcessResult
{
    int exitCode = -1;
    QString stdOut;
    QString stdErr;
    bool timedOut = false;
};

class ProcessRunner
{
public:
    static ProcessResult run(const QString& program,
                             const QStringList& args,
                             const QProcessEnvironment& env,
                             int timeoutMs);
};

#endif // PROCESSRUNNER_H
