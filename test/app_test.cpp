/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <list>

#include <gtest/gtest.h>

#include "myframe/app.h"
#include "myframe/flags.h"

TEST(app, list) {
  std::list<int> l1 = {1, 3, 5, 7};
  std::list<int> l2 = {2, 4, 6, 8};

  l1.insert(l1.end(), l2.begin(), l2.end());
  for (const auto& it : l1) {
    std::cout << it << std::endl;
  }
  for (const auto& it : l2) {
    std::cout << it << std::endl;
  }
}
