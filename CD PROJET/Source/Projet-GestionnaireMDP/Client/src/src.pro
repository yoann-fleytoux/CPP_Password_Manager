#-------------------------------------------------
#
# Project created by QtCreator 2013-11-29T18:18:02
#
#-------------------------------------------------

QT += core gui widgets network

TEMPLATE = lib
CONFIG += staticlib

SOURCES += ComposantPrincipal.cpp \
    ../../Serveur/src/ComposantCryptographie.cpp \
    ComposantReseau.cpp \
    GestionUtilisateurs.cpp \
    GestionGroupes.cpp \
    GestionServeurs.cpp \
    GestionMotsDePasse.cpp \
    ComposantSauvegarde.cpp

HEADERS  += ComposantPrincipal.h \
    ../../Serveur/src/ComposantCryptographie.h \
    ComposantReseau.h \
    GestionUtilisateurs.h \
    GestionGroupes.h \
    GestionServeurs.h \
    GestionMotsDePasse.h \
    ComposantSauvegarde.h

win32 {
FORMS    += ComposantPrincipal_win.ui \
    GestionUtilisateurs_win.ui \
    GestionGroupes_win.ui \
    GestionServeurs_win.ui \
    GestionMotsDePasse_win.ui \
    ComposantSauvegarde_win.ui
}

unix {
FORMS    += ComposantPrincipal_unix.ui \
    GestionUtilisateurs_unix.ui \
    GestionGroupes_unix.ui \
    GestionServeurs_unix.ui \
    GestionMotsDePasse_unix.ui \
    ComposantSauvegarde_unix.ui
}

win32: LIBS += C:/OpenSSL-Win32/lib/libeay32.lib C:/OpenSSL-Win32/lib/ssleay32.lib
unix:  LIBS += -lssl -lcrypto

RESOURCES += \
    Ressources.qrc

