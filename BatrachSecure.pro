QT       += core network

QT       -= gui

TARGET = BtxSecure
CONFIG   += console debug
CONFIG   -= app_bundle
TEMPLATE = app

HEADERS += src/api_client.h src/btx_exception.h src/client_conf.h
SOURCES += src/main.cpp src/btx_exception.cpp src/api_client.cpp src/client_conf.cpp

INCLUDEPATH += ../libaxolotl-qt5/

LIBS += -L../libaxolotl-qt5/ -L../libcurve25519/ -laxolotl -lcurve25519

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -pedantic -std=c++11 -O2 -g -ggdb3 -fno-inline

