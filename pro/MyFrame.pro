TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt core

DEFINES += MYFRAME_DEBUG

TARGET = myframe

DESTDIR = ../bin

OBJECTS_DIR = ../bin

INCLUDEPATH += ../MyFrame/ ../3rd/ ../test/

include(MyFrame.pri)

SOURCES += \
    ../MyFrame/MyMain.cpp

unix {
    QMAKE_LFLAGS += -Wl,-E
    LIBS += -lpthread -ldl -lrt -lm -lboost_program_options -lboost_system -lboost_filesystem
    #QMAKE_CXXFLAGS += -fno-stack-protector -Wno-reorder
}


