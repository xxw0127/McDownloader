#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QString>

class QSettings;

class AppSettings : public QObject
{
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = nullptr, const QString& iniPath = QString());

    QString minecraftDir() const;
    void setMinecraftDir(const QString& path);
    QString javaPath() const;
    void setJavaPath(const QString& path);

private:
    QSettings* m_settings = nullptr;
};

#endif // APPSETTINGS_H
