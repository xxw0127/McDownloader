QT += testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_main

INCLUDEPATH += $$PWD/../src
DEFINES += TEST_FIXTURE_DIR=\\\"$$PWD/fixtures\\\"

include(../src/core.pri)

RESOURCES += $$PWD/../resources/qml.qrc

HEADERS += \
    $$PWD/tst_paths.h \
    $$PWD/tst_checksum.h \
    $$PWD/tst_logger.h \
    $$PWD/tst_version.h \
    $$PWD/tst_versionlistmodel.h \
    $$PWD/tst_download.h \
    $$PWD/tst_jsonfetcher.h \
    $$PWD/tst_mirror.h \
    $$PWD/TestHttpServer.h \
    $$PWD/../src/AppController.h

SOURCES += \
    $$PWD/tst_main.cpp \
    $$PWD/tst_paths.cpp \
    $$PWD/tst_checksum.cpp \
    $$PWD/tst_logger.cpp \
    $$PWD/tst_version.cpp \
    $$PWD/tst_versionlistmodel.cpp \
    $$PWD/tst_download.cpp \
    $$PWD/tst_jsonfetcher.cpp \
    $$PWD/tst_mirror.cpp \
    $$PWD/TestHttpServer.cpp \
    $$PWD/../src/AppController.cpp
