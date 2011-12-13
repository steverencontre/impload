#-------------------------------------------------
#
# Project created by QtCreator 2011-12-07T00:52:29
#
#-------------------------------------------------

QT       += core gui

QMAKE_CXXFLAGS += -std=c++0x

debug: DEFINES += _DEBUG
#DEFINES += $$CONFIG

#CONFIG +=

TARGET = impload
TEMPLATE = app
LIBS += -lgphoto2 -lgphoto2_port -lexiv2

SOURCES += main.cpp\
        MainWindow.cpp \
    Camera.cpp \
    LoaderThread.cpp \
    PreviewGrid.cpp

HEADERS  += MainWindow.h \
    Camera.h \
    LoaderThread.h \
    PreviewGrid.h

FORMS    += MainWindow.ui
