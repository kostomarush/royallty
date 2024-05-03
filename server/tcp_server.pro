#-------------------------------------------------
#
# Project created by QtCreator 2022-05-04T15:55:29
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tcp_server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        monitoringobject.cpp

HEADERS  += mainwindow.h \
    monitoringobject.h

FORMS    += mainwindow.ui
