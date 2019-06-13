set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/CXXService)

add_library(demo SHARED MyDemo.cpp)
add_library(echo_tcp_server SHARED MyEchoSrv.cpp)
add_library(test_tcp_server SHARED MyTestTcpSrv.cpp)

add_library(test_service SHARED MyTestService.cpp)
add_library(test_service2 SHARED MyTestService2.cpp)
add_library(test_service3 SHARED MyTestService3.cpp)

