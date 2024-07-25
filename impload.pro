#-------------------------------------------------
#
# Project created by QtCreator 2011-12-07T00:52:29
#
#-------------------------------------------------

QT       = core gui widgets


debug: DEFINES += _DEBUG
#DEFINES += $$CONFIG

CONFIG += c++latest

TARGET = impload
TEMPLATE = app

LIBS += -L/usr/local/lib64 -L/usr/local/lib
LIBS += -lgphoto2 -lgphoto2_port -lexiv2
LIBS += -lmediainfo
LIBS += -lyaml-cpp

EXIFTOOL = /home/steve/OpenSource/cpp_exiftool

#INCLUDEPATH +=/usr/local/include #/usr/local/include $${EXIFTOOL}/inc

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

#SOURCES += $${EXIFTOOL}/src/ExifTool.cpp $${EXIFTOOL}/src/ExifToolPipe.cpp $${EXIFTOOL}/src/TagInfo.cpp

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
