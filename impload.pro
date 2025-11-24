#-------------------------------------------------
#
# Project created by QtCreator 2011-12-07T00:52:29
#
#-------------------------------------------------

QT       = core gui widgets

#debug: DEFINES += _DEBUG
#DEFINES += $$CONFIG

CONFIG += c++latest

TARGET = impload
TEMPLATE = app

LIBS += -lgphoto2 -lgphoto2_port -lexiv2
LIBS += -lmediainfo
LIBS += -lyaml-cpp

SOURCES += main.cpp \
    Camera.cpp \
    Metadata.cpp \
    Folder.cpp \
    ImageSource.cpp \
    MainWindow.cpp \
    LoaderThread.cpp \
    PreviewGrid.cpp \
    Chooser.cpp \
    CameraWidget.cpp

HEADERS  += MainWindow.h \
    Camera.h \
    Metadata.h \
    Folder.h \
    ImageSource.h \
    LoaderThread.h \
    PreviewGrid.h \
    Chooser.h \
    CameraWidget.h \
    gphoto2.h

FORMS    += MainWindow.ui \
    Chooser.ui
