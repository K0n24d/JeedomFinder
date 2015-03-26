#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T15:34:07
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += static

TARGET = JeedomFinder
TEMPLATE = app

INCLUDEPATH += Bonjour

SOURCES += main.cpp\
    quickstartwizard.cpp \
    intropage.cpp \
    searchpage.cpp \
    advancedsearchpage.cpp \
    pingsearchworker.cpp \
    Bonjour/bonjourserviceresolver.cpp \
    Bonjour/bonjourservicebrowser.cpp \
    bonjoursearchworker.cpp \
    searchworker.cpp \
    host.cpp \
    dnslookupsearchworker.cpp \
    udpsearchworker.cpp

HEADERS  += \
    quickstartwizard.h \
    intropage.h \
    searchpage.h \
    advancedsearchpage.h \
    pingsearchworker.h \
    Bonjour/bonjourserviceresolver.h \
    Bonjour/bonjourservicebrowser.h \
    Bonjour/bonjourrecord.h \
    bonjoursearchworker.h \
    searchworker.h \
    host.h \
    dnslookupsearchworker.h \
    udpsearchworker.h

FORMS    +=

RESOURCES += \
    resources.qrc

linux-g++*:LIBS+=-ldns_sd -lavahi-client -lavahi-common -ldbus-1
win32:LIBS+=-ldnssd
win32:LIBPATH+="C:/Program Files/Bonjour SDK/Lib/Win32"
win32:INCLUDEPATH+="C:/Program Files/Bonjour SDK/Include"

CODECFORSRC = UTF-8
CODECFORTR = UTF-8

defineTest(matches) {
  value = $$1
  regex = $$2
  test = $$replace($${value}, $${regex}, "")
  isEmpty($${test}) {
    return(true)
  } else {
    return(false)
  }
}
 
#linux-g++* {
#   for(lib, LIBS) {
#      # If it's something we don't recognize (neither "-lfoo" nor "-Lfoo") just add it directly
#      !matches(lib, "^-l.*$") {
#         libtemp *= $${lib}
#      # Don't statically link POSIX threading or dlOpen libraries
#      } else:isEqual(lib, "-lpthread") | isEqual(lib, "-ldl") {
#         libtemp *= $${lib}
#      # Ask GCC to find a static version of the library
#      } else {
#         libfile = $$replace(lib, "^-l(.*)$", "lib\\1.a")
#         libloc = $$system("gcc $${LIBS} -print-file-name=$${libfile}")
#         # If it didn't find it, just keep the 
#         isEqual(libloc, $${libfile}) {
#            libtemp *= $${lib}
#         } else {
#            libtemp *= $${libloc}
#         }
#      }
#   }
#   LIBS = $${libtemp}
#   unset(libtemp)
#   unset(libfile)
#   unset(libloc)
#}

CONFIG(release, debug|release) {
#  linux-g++*:QMAKE_POST_LINK=strip $(TARGET) && upx -k --ultra-brute $(TARGET)
  mac*:QMAKE_POST_LINK=macdeployqt JeedomFinder.app $(TARGET) -dmg
}

win32:RC_FILE += \
    icon.rc

mac*:ICON = \
    jeedom.icns

QMAKE_MAC_SDK = macosx10.9
DEFINES *= QT_USE_QSTRINGBUILDER

contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
  DEFINES *= HAS_SSL_SUPPORT
}

TRANSLATIONS += \
    Translations/JeedomFinder_fr.ts \
    Translations/JeedomFinder_de.ts \

RESOURCES += \
    translations.qrc

include(updateqm.pri)
