cmake_minimum_required(VERSION 2.8)
project(myframe)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

option(MYFRAME_DEBUG "debug myframe option" off)
option(MYFRAME_TEST "build myframe test case option" off)
option(MYFRAME_EXAMPLE "build myframe Service folder src option" on)

if(MYFRAME_DEBUG)
    add_definitions(-DMYFRAME_DEBUG)
endif(MYFRAME_DEBUG)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS filesystem system program_options log)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(${PROJECT_SOURCE_DIR}/myframe)

set(MYFRAME_SRCS 
    ${PROJECT_SOURCE_DIR}/myframe/MyApp.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyCUtils.c
    ${PROJECT_SOURCE_DIR}/myframe/MyContext.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyEvent.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyFrame.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyHandleMgr.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyList.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyLog.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyModules.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyMsg.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyObj.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyThread.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyWorker.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MySocksMgr.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MySock.cpp
    ${PROJECT_SOURCE_DIR}/myframe/MyTimerTask.cpp
)

add_subdirectory(myframe)
add_subdirectory(service)

if(MYFRAME_EXAMPLE)
    add_subdirectory(examples)
endif(MYFRAME_EXAMPLE)

if(MYFRAME_TEST)
    add_subdirectory(test)
endif(MYFRAME_TEST)


