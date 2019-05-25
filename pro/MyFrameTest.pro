TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt core

TARGET = myframe_test

DESTDIR = ../bin

OBJECTS_DIR = ../bin

INCLUDEPATH += ../MyFrame/ ../3rd/ ../test/

DEFINES += GTEST

include(MyFrame.pri)

SOURCES += \
    ../test/MyObj_test.cpp \
    ../test/test_main.cpp \
    ../test/MyList_test.cpp \
    ../test/MyLog_test.cpp \
    ../test/MyEvent_test.cpp \
    ../test/MyCUtils_test.cpp \
    ../test/MyThread_test.cpp \
    ../test/MyFrame_test.cpp

unix {
    LIBS += -ldl -lrt
    LIBS += -pthread
    LIBS += -lgtest_main -lgtest
    #QMAKE_CXXFLAGS += -fno-stack-protector -Wno-reorder
}

