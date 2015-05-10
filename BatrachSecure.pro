QT       += core network

QT       -= gui

TARGET = BtxSecure
CONFIG   += console debug
CONFIG   -= app_bundle
TEMPLATE = app

# HEADERS += src/api_client.h src/btx_exception.h src/client_conf.h src/pre_key_store.h
SOURCES += src/main.cpp

INCLUDEPATH += ../libaxolotl-qt5/ ../libtextsecure-qt5/

LIBS += -L../libcurve25519/ -L../libaxolotl-qt5/ -L ../libtextsecure-qt5/ -lcurve25519 -laxolotl -ltextsecureqt5

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -pedantic -std=c++11 -O2 -g -ggdb3 -fno-inline

