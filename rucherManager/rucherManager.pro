#-------------------------------------------------
#
# Project created by QtCreator 2016-04-10T08:04:28
#
#-------------------------------------------------

QT       += core gui webkit network sql

greaterThan(QT_MAJOR_VERSION, 4):QT += widgets webkitwidgets

TARGET = rucherManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connexion.cpp \
    map.cpp \
    addressfinder.cpp

HEADERS  += mainwindow.h \
    connexion.h \
    initialisation.h \
    map.h \
    addressfinder.h

FORMS    += mainwindow.ui \
    connexion.ui \
    map.ui \
    addressfinder.ui
