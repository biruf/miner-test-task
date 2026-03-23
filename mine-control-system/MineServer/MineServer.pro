QT += core network sql
QT -= gui

CONFIG += c++11
TARGET = mineserver
TEMPLATE = app

INCLUDEPATH += $$PWD/../common

SOURCES += \
    main.cpp \
    mineserver.cpp \
    miner.cpp \
    logger.cpp \
    $$PWD/../common/messages.cpp

HEADERS += \
    mineserver.h \
    miner.h \
    logger.h \
    $$PWD/../common/messages.h
