TEMPLATE = app
TARGET = PCL2Downloader

QT += quick quickcontrols2 widgets

CONFIG += c++17
CONFIG -= app_bundle

include(src/core.pri)

HEADERS += \
    $$PWD/src/AppController.h

SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/AppController.cpp

RESOURCES += \
    $$PWD/resources/qml.qrc
