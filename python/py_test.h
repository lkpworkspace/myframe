/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <iostream>

namespace myframe {

class PyTestObj {
 public:
  PyTestObj() {
    std::cout << "PyTestObj constructor" << std::endl;
  }
  ~PyTestObj() {
    std::cout << "PyTestObj destructor" << std::endl;
  }

  int GetValue() const {
    return 42;
  }
};

class PyTest {
 public:
  PyTest() {
    std::cout << "PyTest constructor" << std::endl;
  }
  ~PyTest() {
    std::cout << "PyTest destructor" << std::endl;
  }

  PyTestObj* CreateTestObj() {
    return new PyTestObj();
  }
};

}  // namespace myframe
