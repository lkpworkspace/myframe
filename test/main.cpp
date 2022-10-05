/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <gtest/gtest.h>

class Environment : public testing::Environment
{
public:

    virtual void SetUp()
    {
        std::cout << "Environment SetUP" << std::endl;
    }

    virtual void TearDown()
    {
        std::cout << "Environment TearDown" << std::endl;
    }
};

int main(int argc, char** argv)
{
    testing::AddGlobalTestEnvironment(new Environment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
