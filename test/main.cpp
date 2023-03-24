/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <gtest/gtest.h>

class Environment : public testing::Environment {
 public:
  void SetUp() override { std::cout << "Environment SetUP" << std::endl; }

  void TearDown() override { std::cout << "Environment TearDown" << std::endl; }
};

int main(int argc, char** argv) {
  testing::AddGlobalTestEnvironment(new Environment);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
