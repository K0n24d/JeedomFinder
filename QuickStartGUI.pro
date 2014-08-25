#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T15:34:07
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += static

TARGET = QuickStartGUI
TEMPLATE = app

INCLUDEPATH += Bonjour

SOURCES += main.cpp\
    quickstartwizard.cpp \
    intropage.cpp \
    searchpage.cpp \
    advancedsearchpage.cpp \
    conclusionpage.cpp \
    pingsearchworker.cpp \
    Bonjour/bonjourserviceresolver.cpp \
    Bonjour/bonjourservicebrowser.cpp \
    bonjoursearchworker.cpp

HEADERS  += \
    quickstartwizard.h \
    intropage.h \
    searchpage.h \
    advancedsearchpage.h \
    conclusionpage.h \
    pingsearchworker.h \
    Bonjour/bonjourserviceresolver.h \
    Bonjour/bonjourservicebrowser.h \
    Bonjour/bonjourrecord.h \
    bonjoursearchworker.h

FORMS    +=

RESOURCES += \
    resources.qrc

unix:LIBS+=-ldns_sd
win32:LIBS+=-ldnssd

CODECFORSRC = UTF-8
CODECFORTR = UTF-8
