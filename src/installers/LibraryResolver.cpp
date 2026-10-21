#include "installers/LibraryResolver.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QSet>

#include "core/JsonRules.h"
#include "download/MirrorResolver.h"

namespace
{
struct MavenCoordinate
{
    QString group;
    QString artifact;
    QString version;
    QString classifier;
    QString path;
    bool valid = false;
};

QString expandPlaceholders(QString value)
{
    value.replace(QStringLiteral("${arch}"), QStringLiteral("64"));
    return value;
}

MavenCoordinate parseMavenName(const QString& rawName)
{
    MavenCoordinate coordinate;
    const QString name = expandPlaceholders(rawName);
    const QStringList parts = name.split(QLatin1Char(':'));
    if (parts.size() < 3)
        return coordinate;

    coordinate.group = parts.at(0);
    coordinate.artifact = parts.at(1);
    coordinate.version = parts.at(2);
    if (parts.size() >= 4)
        coordinate.classifier = parts.at(3);

    QString groupPath = coordinate.group;
    groupPath.replace(QLatin1Char('.'), QLatin1Char('/'));
    QString fileName = coordinate.artifact + QLatin1Char('-') + coordinate.version;
    if (!coordinate.classifier.isEmpty())
        fileName += QLatin1Char('-') + coordinate.classifier;
    fileName += QStringLiteral(".jar");

    coordinate.path = groupPath + QLatin1Char('/') + coordinate.artifact + QLatin1Char('/') +
                      coordinate.version + QLatin1Char('/') + fileName;
    coordinate.valid = true;
    return coordinate;
}

qint64 jsonSize(const QJsonObject& object)
{
    if (!object.contains(QStringLiteral("size")))
        return -1;
    return object.value(QStringLiteral("size")).toVariant().toLongLong();
}

LibraryFile makeFile(const QString& rawPath, const QString& rawUrl, const QJsonObject& object, bool isNatives)
{
    LibraryFile file;
    const QString path = expandPlaceholders(rawPath);
    const QString url = expandPlaceholders(rawUrl);
    file.relativePath = QStringLiteral("libraries/") + path;
    file.size = jsonSize(object);
    file.sha1 = object.value(QStringLiteral("sha1")).toString().toLatin1();
    file.isNatives = isNatives;

    const QUrl official = url.isEmpty() ? QUrl(QStringLiteral("https://libraries.minecraft.net/") + path)
                                        : QUrl(url);
    file.urls = MirrorResolver::libraryUrls(path, official);
    return file;
}
}

QVector<LibraryFile> LibraryResolver::resolve(const QJsonObject& versionJson,
                                              const QString& osName,
                                              const QString& arch)
{
    QVector<LibraryFile> result;
    QSet<QString> seen;

    const QJsonArray libraries = versionJson.value(QStringLiteral("libraries")).toArray();
    for (const QJsonValue& value : libraries)
    {
        const QJsonObject library = value.toObject();
        if (!JsonRules::libraryAllowed(library, osName, arch))
            continue;

        const MavenCoordinate coordinate = parseMavenName(library.value(QStringLiteral("name")).toString());
        if (!coordinate.valid)
            continue;

        const QJsonObject downloads = library.value(QStringLiteral("downloads")).toObject();
        const QJsonObject artifact = downloads.value(QStringLiteral("artifact")).toObject();
        const QJsonObject natives = library.value(QStringLiteral("natives")).toObject();

        auto add = [&](LibraryFile file) {
            const QString key = file.relativePath + (file.isNatives ? QStringLiteral("|n") : QStringLiteral("|a"));
            if (!seen.contains(key))
            {
                seen.insert(key);
                result.append(file);
            }
        };

        bool artifactAdded = false;
        if (!artifact.isEmpty())
        {
            QString path = artifact.value(QStringLiteral("path")).toString();
            if (path.isEmpty())
                path = coordinate.path;
            add(makeFile(path, artifact.value(QStringLiteral("url")).toString(), artifact, false));
            artifactAdded = true;
        }

        if (!natives.isEmpty())
        {
            const QString classifier = natives.value(osName).toString();
            if (classifier.isEmpty())
                continue;

            const QJsonObject classifierObject = downloads.value(QStringLiteral("classifiers")).toObject()
                                                     .value(classifier).toObject();
            QString path = classifierObject.value(QStringLiteral("path")).toString();
            if (path.isEmpty())
            {
                QString fileName = coordinate.artifact + QLatin1Char('-') + coordinate.version +
                                   QLatin1Char('-') + classifier + QStringLiteral(".jar");
                QString groupPath = coordinate.group;
                groupPath.replace(QLatin1Char('.'), QLatin1Char('/'));
                path = groupPath + QLatin1Char('/') + coordinate.artifact + QLatin1Char('/') +
                       coordinate.version + QLatin1Char('/') + fileName;
            }

            add(makeFile(path,
                         classifierObject.value(QStringLiteral("url")).toString(),
                         classifierObject,
                         true));
        }
        else if (!artifactAdded)
        {
            add(makeFile(coordinate.path, QString(), QJsonObject(), false));
        }
    }

    return result;
}
