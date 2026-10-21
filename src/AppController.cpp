#include "AppController.h"

AppController::AppController(QObject* parent)
    : QObject(parent)
{
}

QString AppController::appName() const
{
    return QStringLiteral("PCL2 下载器");
}
