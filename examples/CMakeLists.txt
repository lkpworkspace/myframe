set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/CXXService)

add_library(demo SHARED MyDemo.cpp)
add_library(test_timer SHARED MyTestTimer.cpp)
add_library(echo_tcp_server SHARED MyEchoSrv.cpp)
