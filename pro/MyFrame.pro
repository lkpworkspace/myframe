TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt core

DEFINES += MYFRAME_DEBUG
#DEFINES += BOOST_LOG_DYN_LINK
DEFINES += BOOST_ALL_DYN_LINK
TARGET = myframe

DESTDIR = ../bin

OBJECTS_DIR = ../bin

INCLUDEPATH += ../MyFrame/ ../3rd/ ../test/
INCLUDEPATH += $$system(echo ${BOOST_INCLUDE_DIR})

include(MyFrame.pri)

SOURCES += \
    ../MyFrame/MyMain.cpp

unix {
    QMAKE_LFLAGS += -Wl,-E
    LIBS += -L$$system(echo ${BOOST_LIB_DIR})
    LIBS += -lpthread -pthread -ldl -lrt -lm -lboost_program_options -lboost_system -lboost_filesystem -lboost_log_setup -lboost_log -lboost_thread
    #QMAKE_CXXFLAGS += -fno-stack-protector -Wno-reorder
}


