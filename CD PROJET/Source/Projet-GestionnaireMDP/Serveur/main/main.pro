#-------------------------------------------------
#
# Project created by QtCreator 2013-11-29T18:18:02
#
#-------------------------------------------------

QT += core network sql
QT -= gui

TEMPLATE = app
CONFIG += console
SOURCES += main.cpp
INCLUDEPATH += ../src/
DEPENDPATH += $${INCLUDEPATH} # force rebuild if the headers change

# link against src lib
_SRC_LIB = ../
CONFIG(debug, debug|release) {
    win32: _SRC_LIB = $$join(_SRC_LIB,,,src/debug/libsrc.a)
    unix: _SRC_LIB = $$join(_SRC_LIB,,,src/libsrc.a)
} else {
    win32: _SRC_LIB = $$join(_SRC_LIB,,,src/release/libsrc.a)
    unix: _SRC_LIB = $$join(_SRC_LIB,,,src/libsrc.a)
}
LIBS += $${_SRC_LIB}
PRE_TARGETDEPS += $${_SRC_LIB}

win32: LIBS += C:/OpenSSL-Win32/lib/libeay32.lib C:/OpenSSL-Win32/lib/ssleay32.lib
unix:  LIBS += -lssl -lcrypto
