QT += core network
QT -= gui

CONFIG += c++11
TARGET = mineclient
TEMPLATE = app

INCLUDEPATH += $$PWD/../common

SOURCES += \
    main.cpp \
    mineclient.cpp \
    $$PWD/../common/messages.cpp

HEADERS += \
    mineclient.h \
    $$PWD/../common/messages.h
