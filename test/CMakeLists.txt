cmake_minimum_required(VERSION 3.10)

AUX_SOURCE_DIRECTORY(. TEST_SRCS)
ADD_EXECUTABLE(myframe_test ${TEST_SRCS})
target_link_libraries(myframe_test
    GTest::GTest
    GTest::Main
    myframe
)

INSTALL(TARGETS myframe_test
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_TEST_DIR}
)