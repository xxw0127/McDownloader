#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)

public:
    explicit AppController(QObject* parent = nullptr);

    QString appName() const;
};

#endif // APPCONTROLLER_H
