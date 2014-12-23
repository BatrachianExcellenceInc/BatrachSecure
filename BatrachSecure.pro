QT       += core network

QT       -= gui

TARGET = BtxSecure
CONFIG   += console debug
CONFIG   -= app_bundle
TEMPLATE = app

HEADERS += src/api_client.h src/btx_exception.h src/client_conf.h
SOURCES += src/main.cpp src/btx_exception.cpp src/api_client.cpp src/client_conf.cpp

INCLUDEPATH += ../libaxolotl-cpp/src

LIBS += -L../libaxolotl-cpp/lib/ -L../libaxolotl-cpp/curve25519/ -laxolotlcpp -lcurve25519-donna

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -pedantic -std=c++11 -O2 -g -ggdb3 -fno-inline

