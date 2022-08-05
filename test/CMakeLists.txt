AUX_SOURCE_DIRECTORY(. TEST_SRCS)

ADD_EXECUTABLE(myframe_test ${TEST_SRCS})
target_link_libraries(myframe_test
    GTest::GTest
    GTest::Main
    myframe
)