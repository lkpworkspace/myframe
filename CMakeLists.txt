cmake_minimum_required(VERSION 2.8)
project(MyFrame)

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
)

# add_subdirectory(Service)
add_subdirectory(MyFrame)
add_subdirectory(examples)
# add_subdirectory(test)


