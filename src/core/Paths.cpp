#include "core/Paths.h"

#include <QDir>
#include <QFileInfo>

namespace
{
const int kMaxInstanceNameLength = 100;

QString normalizeSeparators(const QString& path)
{
    QString result = path;
    result.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return result;
}

bool isInvalidNameChar(QChar c)
{
    if (c.unicode() < 0x20)
        return true;
    switch (c.unicode())
    {
    case '/':
    case ':':
    case '*':
    case '?':
    case '"':
    case '<':
    case '>':
    case '|':
        return true;
    default:
        return false;
    }
}
}

bool Paths::isSafeRelativePath(const QString& path)
{
    if (path.isEmpty())
        return false;

    const QString normalized = normalizeSeparators(path);
    if (normalized.startsWith(QLatin1Char('/')))
        return false;
    if (normalized.contains(QLatin1Char(':')))
        return false;

    const QStringList segments = normalized.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (segments.isEmpty())
        return false;

    for (const QString& segment : segments)
    {
        if (segment == QLatin1String("..") || segment == QLatin1String("."))
            return false;
        for (const QChar c : segment)
        {
            if (c.unicode() < 0x20)
                return false;
        }
    }
    return true;
}

QString Paths::sanitizeInstanceName(const QString& name)
{
    QString result;
    result.reserve(name.size());
    for (const QChar c : name)
        result.append(isInvalidNameChar(c) ? QLatin1Char('_') : c);

    while (result.endsWith(QLatin1Char(' ')) || result.endsWith(QLatin1Char('.')))
        result.chop(1);

    if (result.isEmpty())
        result = QStringLiteral("instance");

    if (result.size() > kMaxInstanceNameLength)
        result = result.left(kMaxInstanceNameLength);

    return result;
}

bool Paths::isWithin(const QString& root, const QString& candidate)
{
    if (root.isEmpty() || candidate.isEmpty())
        return false;

    QString cleanRoot = QDir::cleanPath(normalizeSeparators(root));
    QString cleanCandidate = QDir::cleanPath(normalizeSeparators(candidate));

#ifdef Q_OS_WIN
    cleanRoot = cleanRoot.toLower();
    cleanCandidate = cleanCandidate.toLower();
#endif

    if (cleanCandidate == cleanRoot)
        return true;
    return cleanCandidate.startsWith(cleanRoot + QLatin1Char('/'));
}

QString Paths::joinWithin(const QString& root, const QString& relative, bool* ok)
{
    if (!isSafeRelativePath(relative))
    {
        if (ok != nullptr)
            *ok = false;
        return QString();
    }

    const QString result = QDir(normalizeSeparators(root)).filePath(normalizeSeparators(relative));
    if (!isWithin(root, result))
    {
        if (ok != nullptr)
            *ok = false;
        return QString();
    }

    if (ok != nullptr)
        *ok = true;
    return result;
}
