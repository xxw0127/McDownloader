QT += core network concurrent

CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = installer-cli

INCLUDEPATH += $$PWD/../../src

include($$PWD/../../src/core.pri)

SOURCES += $$PWD/main.cpp
