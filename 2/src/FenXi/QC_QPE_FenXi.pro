#-------------------------------------------------
#
# Project created by QtCreator 2014-09-25T19:12:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QC_QPE_FenXi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    LogWriter.h

FORMS    += mainwindow.ui


unix:!macx: LIBS += ./libLogWriter.so
win32: LIBS +=./LogWriter.dll
