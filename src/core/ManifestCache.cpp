#include "core/ManifestCache.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

#include "platform/PlatformPaths.h"

namespace
{
void setError(QString* error, const QString& message)
{
    if (error != nullptr)
        *error = message;
}

bool isManifest(const QByteArray& body, QString* error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        setError(error, QStringLiteral("版本清单缓存不是有效 JSON：%1").arg(parseError.errorString()));
        return false;
    }
    if (!document.object().value(QStringLiteral("versions")).isArray())
    {
        setError(error, QStringLiteral("版本清单缓存缺少 versions 数组"));
        return false;
    }
    return true;
}
}

QString ManifestCache::defaultPath()
{
    return QDir(PlatformPaths::cacheDir()).filePath(QStringLiteral("version_manifest.json"));
}

bool ManifestCache::save(const QByteArray& body, QString* error)
{
    return saveTo(defaultPath(), body, error);
}

bool ManifestCache::load(QByteArray* body, QString* error)
{
    return loadFrom(defaultPath(), body, error);
}

bool ManifestCache::saveTo(const QString& path, const QByteArray& body, QString* error)
{
    if (!isManifest(body, error))
        return false;

    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
    {
        setError(error, QStringLiteral("无法创建版本清单缓存目录：%1").arg(QFileInfo(path).absolutePath()));
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        setError(error, QStringLiteral("无法写入版本清单缓存：%1").arg(path));
        return false;
    }
    if (file.write(body) != body.size() || !file.commit())
    {
        setError(error, QStringLiteral("提交版本清单缓存失败：%1").arg(path));
        return false;
    }

    if (error != nullptr)
        error->clear();
    return true;
}

bool ManifestCache::loadFrom(const QString& path, QByteArray* body, QString* error)
{
    if (body == nullptr)
    {
        setError(error, QStringLiteral("缓存输出参数为空"));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        setError(error, QStringLiteral("版本清单缓存不存在：%1").arg(path));
        return false;
    }

    const QByteArray data = file.readAll();
    if (!isManifest(data, error))
        return false;

    *body = data;
    if (error != nullptr)
        error->clear();
    return true;
}
