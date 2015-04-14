#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T15:34:07
#
#-------------------------------------------------

#GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
#DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
#VERSION = $$GIT_VERSION
#linux {
#    VERSION ~= s/-\d+-g[a-f0-9]{6,}//
#    VERSION ~= s/v//
#}
#GIT_VERSION = $$VERSION
include(gitversion.pri)

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# UPX Compression on linux (qmake "CONFIG+=upx")
# Build with avahi/mdns support (qmake "CONFIG+=zeroconf")
# Static build (qmake "CONFIG+=static")

TARGET = JeedomFinder
TEMPLATE = app

SOURCES += \
    main.cpp\
    quickstartwizard.cpp \
    intropage.cpp \
    searchpage.cpp \
    advancedsearchpage.cpp \
    pingsearchworker.cpp \
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
    searchworker.h \
    host.h \
    dnslookupsearchworker.h \
    udpsearchworker.h

FORMS    +=

RESOURCES += \
    resources.qrc

# Build with avahi/mdns support (qmake "CONFIG+=zeroconf")
zeroconf {
    INCLUDEPATH += Bonjour
    SOURCES += \
        Bonjour/bonjourserviceresolver.cpp \
        Bonjour/bonjourservicebrowser.cpp \
        bonjoursearchworker.cpp
    HEADERS  += \
        Bonjour/bonjourserviceresolver.h \
        Bonjour/bonjourservicebrowser.h \
        Bonjour/bonjourrecord.h \
        Bonjour/dns_sd_late.h \
        bonjoursearchworker.h \

    DEFINES+=WITH_ZEROCONF
    unix:DEFINES+=DNS_SD_LIB_NAME=\\\"dns_sd\\\"
    win*:DEFINES+=DNS_SD_LIB_NAME=\\\"dnssd\\\"
}

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

static:linux-g++* {
   for(lib, LIBS) {
      # If it's something we don't recognize (neither "-lfoo" nor "-Lfoo") just add it directly
      !matches(lib, "^-l.*$") {
         libtemp *= $${lib}
      # Don't statically link POSIX threading or dlOpen libraries
      } else:isEqual(lib, "-lpthread") | isEqual(lib, "-ldl") {
         libtemp *= $${lib}
      # Ask GCC to find a static version of the library
      } else {
         libfile = $$replace(lib, "^-l(.*)$", "lib\\1.a")
         libloc = $$system("gcc $${LIBS} -print-file-name=$${libfile}")
         # If it didn't find it, just keep the 
         isEqual(libloc, $${libfile}) {
            libtemp *= $${lib}
         } else {
            libtemp *= $${libloc}
         }
      }
   }
   LIBS = $${libtemp}
   unset(libtemp)
   unset(libfile)
   unset(libloc)
}

CONFIG(release, debug|release) {
  # UPX Compression on linux (qmake "CONFIG+=upx")
  upx:linux-g++*:QMAKE_POST_LINK=strip $(TARGET) && upx -k --ultra-brute $(TARGET)

  mac*:QMAKE_POST_LINK=macdeployqt JeedomFinder.app $(TARGET) -dmg
}

win*:RC_FILE += \
    icon.rc

mac*:ICON = \
    jeedom.icns

QMAKE_MAC_SDK = macosx10.9
DEFINES *= QT_USE_QSTRINGBUILDER

contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
  DEFINES *= HAS_SSL_SUPPORT
}

include(translations.pri)
