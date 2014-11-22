QT       += core network

QT       -= gui

TARGET = BtxSecure
CONFIG   += console debug
CONFIG   -= app_bundle
TEMPLATE = app

HEADERS += src/api_client.h
SOURCES += src/main.cpp src/api_client.cpp

QMAKE_CXXFLAGS += -std=c++11

