QT += core gui widgets network
CONFIG += c++11

TARGET = chat_client
TEMPLATE = app

SOURCES += \
    main.cpp \
    logindialog.cpp \
    chatwindow.cpp \
    logviewer.cpp

HEADERS += \
    logindialog.h \
    chatwindow.h \
    logviewer.h
