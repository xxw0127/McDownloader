#ifndef STAGINGAREA_H
#define STAGINGAREA_H

#include <QString>

class StagingArea
{
public:
    static StagingArea create(const QString& targetRoot, QString* error);

    QString path() const;
    QString minecraftPath() const;
    bool mergeInto(QString* error);
    void cleanup();

private:
    StagingArea(const QString& path, const QString& minecraftPath);

    QString m_path;
    QString m_minecraftPath;
};

#endif // STAGINGAREA_H
