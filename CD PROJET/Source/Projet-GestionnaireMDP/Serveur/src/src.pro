#-------------------------------------------------
#
# Project created by QtCreator 2013-11-29T18:18:02
#
#-------------------------------------------------

QT += core network sql
QT -= gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += console

SOURCES += ComposantPrincipal.cpp \
    ComposantCryptographie.cpp \
    ComposantReseau.cpp \
    ComposantReseauConnexion.cpp \
    ComposantBaseDeDonnees.cpp \
    ComposantDocumentation.cpp

HEADERS  += ComposantPrincipal.h \
    ComposantCryptographie.h \
    ComposantReseau.h \
    ComposantReseauConnexion.h \
    ComposantBaseDeDonnees.h \
    ComposantDocumentation.h

win32: LIBS += C:/OpenSSL-Win32/lib/libeay32.lib C:/OpenSSL-Win32/lib/ssleay32.lib
unix:  LIBS += -lssl -lcrypto

RESOURCES +=
