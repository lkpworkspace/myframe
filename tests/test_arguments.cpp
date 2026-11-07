/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <iostream>
#include "myframe/arguments.h"
#include "myframe/common.h"

#include "test_config.h"

int main() {
  myframe::Arguments args;
  auto root_path = myframe::Common::GetWorkRoot();
  auto filepath = root_path / "test_data" / "test_arguments.json";
  if (!args.Load(filepath.string())) {
    return -1;
  }
  std::cout << "load args: " << args.DebugString() << std::endl;
  args.SetStr("key3", "world");
  if (!args.Save(filepath.string())) {
    return -2;
  }
  return 0;
}
