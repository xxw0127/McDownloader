#include "installers/JavaLocator.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include "installers/ProcessRunner.h"
#include "platform/PlatformProcess.h"

namespace
{
QStringList findJavaUnder(const QString& root)
{
    QStringList result;
    if (root.isEmpty() || !QFileInfo::exists(root))
        return result;

    QDirIterator it(root,
                    {PlatformProcess::javaExecutableName()},
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
        result.append(it.next());
    return result;
}

void appendUnique(QStringList& paths, const QString& path)
{
    if (path.isEmpty() || !QFileInfo::exists(path))
        return;
    const QString canonical = QFileInfo(path).canonicalFilePath();
    if (canonical.isEmpty())
        return;
    for (const QString& existing : paths)
    {
        if (QFileInfo(existing).canonicalFilePath() == canonical)
            return;
    }
    paths.append(canonical);
}
}

JavaInfo JavaLocator::parseVersionOutput(const QString& output)
{
    JavaInfo info;
    info.versionText = output.trimmed();
    info.is64Bit = output.contains(QStringLiteral("64-Bit")) || output.contains(QStringLiteral("64-bit"));

    QRegularExpression versionExpression(QStringLiteral("version\\s+\"([^\"]+)\""));
    auto match = versionExpression.match(output);
    if (!match.hasMatch())
    {
        versionExpression.setPattern(QStringLiteral("version\\s+([0-9][^\\s]*)"));
        match = versionExpression.match(output);
    }
    if (!match.hasMatch())
        return info;

    const QString version = match.captured(1);
    const QStringList parts = version.split(QLatin1Char('.'));
    if (parts.isEmpty())
        return info;

    bool ok = false;
    if (parts.first() == QLatin1String("1") && parts.size() >= 2)
    {
        info.major = parts.at(1).toInt(&ok);
    }
    else
    {
        info.major = parts.first().toInt(&ok);
    }

    if (!ok)
        info.major = 0;
    return info;
}

QVector<JavaInfo> JavaLocator::scan(const QString& minecraftDir)
{
    QStringList candidates;
    for (const QString& candidate : findJavaUnder(QDir(minecraftDir).filePath(QStringLiteral("runtime"))))
        appendUnique(candidates, candidate);

    const QString appData = qEnvironmentVariable("APPDATA");
    if (!appData.isEmpty())
    {
        for (const QString& candidate : findJavaUnder(QDir(appData).filePath(QStringLiteral(".minecraft/runtime"))))
            appendUnique(candidates, candidate);
    }

    const QString javaHome = qEnvironmentVariable("JAVA_HOME");
    if (!javaHome.isEmpty())
        appendUnique(candidates, QDir(javaHome).filePath(QStringLiteral("bin/") + PlatformProcess::javaExecutableName()));

    const QString pathJava = QStandardPaths::findExecutable(QStringLiteral("java"));
    appendUnique(candidates, pathJava);

    for (const QString& root : PlatformProcess::commonJavaRoots())
    {
        for (const QString& candidate : findJavaUnder(root))
            appendUnique(candidates, candidate);
    }

    QVector<JavaInfo> result;
    for (const QString& candidate : candidates)
    {
        const ProcessResult run = ProcessRunner::run(candidate, {QStringLiteral("-version")}, QProcessEnvironment(), 5000);
        JavaInfo info = parseVersionOutput(run.stdOut + run.stdErr);
        if (info.major <= 0)
            continue;
        info.path = candidate;
        result.append(info);
    }
    return result;
}

JavaInfo JavaLocator::select(const QVector<JavaInfo>& candidates, int requiredMajor)
{
    JavaInfo selected;
    for (const JavaInfo& candidate : candidates)
    {
        if (candidate.major < requiredMajor || !candidate.is64Bit)
            continue;
        if (selected.path.isEmpty() || candidate.major > selected.major)
            selected = candidate;
    }
    return selected;
}

JavaInfo JavaLocator::selectForInstaller(const QVector<JavaInfo>& candidates, int requiredMajor)
{
    JavaInfo exact;
    JavaInfo lowestCompatible;
    for (const JavaInfo& candidate : candidates)
    {
        if (candidate.major < requiredMajor || !candidate.is64Bit)
            continue;
        if (candidate.major == requiredMajor)
        {
            exact = candidate;
            break;
        }
        if (lowestCompatible.path.isEmpty() || candidate.major < lowestCompatible.major)
            lowestCompatible = candidate;
    }
    return exact.path.isEmpty() ? lowestCompatible : exact;
}
