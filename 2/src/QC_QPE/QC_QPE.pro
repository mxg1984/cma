#-------------------------------------------------
#
# Project created by QtCreator 2014-03-01T15:26:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QC_QPE
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    inputdialog.cpp \
    tabledialog.cpp

HEADERS  += mainwindow.h \
    qconfigdata.h \
    inputdialog.h \
    tabledialog.h \
    Common.h \
    LogWriter.h


unix:!macx: LIBS += ./libLogWriter.so
win32: LIBS +=./LogWriter.dll

FORMS    += mainwindow.ui \
    tabledialog.ui

RC_FILE = myapp.rc


