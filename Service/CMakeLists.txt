set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/CXXService)

add_library(demo SHARED MyDemo.cpp)
add_library(test_service SHARED MyTestService.cpp)
add_library(test_service2 SHARED MyTestService2.cpp)
add_library(test_service3 SHARED MyTestService3.cpp)
add_library(test_tcp_sock SHARED MyTCPSockTest.cpp)
