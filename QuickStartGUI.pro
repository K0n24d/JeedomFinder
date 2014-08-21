#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T15:34:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickStartGUI
TEMPLATE = app


SOURCES += main.cpp\
    quickstartwizard.cpp \
    intropage.cpp \
    searchpage.cpp \
    advancedsearchpage.cpp \
    conclusionpage.cpp

HEADERS  += \
    quickstartwizard.h \
    intropage.h \
    searchpage.h \
    advancedsearchpage.h \
    conclusionpage.h

FORMS    +=

RESOURCES += \
    resources.qrc
