cmake_minimum_required(VERSION 2.8)
project(MyFrame)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

option(MYFRAME_DEBUG "debug myframe option" off)
option(MYFRAME_TEST "build myframe test case option" off)
option(MYFRAME_EXAMPLE "build myframe Service folder src option" on)

if(MYFRAME_DEBUG)
    add_definitions(-DMYFRAME_DEBUG)
endif(MYFRAME_DEBUG)

include_directories(${PROJECT_SOURCE_DIR}/MyFrame)

set(MYFRAME_SRCS 
    ${PROJECT_SOURCE_DIR}/MyFrame/MyApp.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyCUtils.c
    ${PROJECT_SOURCE_DIR}/MyFrame/MyContext.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyEvent.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyFrame.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyHandleMgr.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyList.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyLog.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyModules.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyMsg.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyObj.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyThread.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyWorker.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MySocksMgr.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MySock.cpp
    ${PROJECT_SOURCE_DIR}/MyFrame/MyTimerTask.cpp
)

add_subdirectory(MyFrame)
add_subdirectory(Service)

if(MYFRAME_EXAMPLE)
    add_subdirectory(examples)
endif(MYFRAME_EXAMPLE)

if(MYFRAME_TEST)
    add_subdirectory(test)
endif(MYFRAME_TEST)


