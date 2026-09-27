QT += core network
CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = qt_tcp_server
TEMPLATE = app

INCLUDEPATH += . functions

SOURCES += \
    main.cpp \
    myserver.cpp \
    functions/logger.cpp \
    functions/protocol.cpp \
    functions/client_handler.cpp

HEADERS += \
    myserver.h \
    functions/logger.h \
    functions/protocol.h \
    functions/client_handler.h
