#include <gtest/gtest.h>
#include "MyCUtils.h"


// TODO ...
TEST(MyCUtils, all_test)
{
    // test random
    uint8_t min = 10, max = 200;
    EXPECT_GE(max, my_random_num(min,max));
    EXPECT_LE(min, my_random_num(min,max));
}
