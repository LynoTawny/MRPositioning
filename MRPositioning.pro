#-------------------------------------------------
#
# Project created by QtCreator 2016-09-28T17:45:57
#
#-------------------------------------------------

QT       += core gui webkitwidgets widgets sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ./eigen

TARGET = MRPositioning
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    drivetestitem.cpp \
    basedbhandler.cpp \
    position_cal/aoa_toa.cpp \
    position_cal/field_func.cpp \
    position_cal/oku_hata.cpp

HEADERS  += mainwindow.h \
    drivetestitem.h \
    mro_type.h \
    basedbhandler.h \
    position_cal/aoa_toa.h \
    position_cal/field_func.h \
    position_cal/oku_hata.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    MRPositioning.pro.user \
