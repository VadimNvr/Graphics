#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T13:00:23
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Practicum_1
TEMPLATE = app


SOURCES += src/model.cpp \
           modellistener.cpp\
           pluginmanager.cpp \
           externals/EasyBMP/src/EasyBMP.cpp \
           src/io.cpp \
           main.cpp\
           mainwindow.cpp

HEADERS  += mainwindow.h \
    externals/EasyBMP/include/EasyBMP.h \
    externals/EasyBMP/include/EasyBMP_BMP.h \
    externals/EasyBMP/include/EasyBMP_DataStructures.h \
    externals/EasyBMP/include/EasyBMP_VariousBMPutilities.h \
    include/io.h \
    include/matrix.h \
    include/matrix.hpp \
    src/MyObject.h \
    modellistener.h \
    src/model.h \
    pluginmanager.h

FORMS    += mainwindow.ui
CONFIG += static

