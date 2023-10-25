/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/log.h"

int main() {
  auto root = myframe::Common::GetWorkRoot();
  LOG(INFO) << "work root is " << root.string();

  auto lib_path = myframe::Common::GetAbsolutePath("lib");
  LOG(INFO) << "lib path is " << lib_path.string();

  auto root_files = myframe::Common::GetDirFiles(root.string());
  LOG(INFO) << "root dir files:";
  for (size_t i = 0; i < root_files.size(); ++i) {
    LOG(INFO) << "  " << root_files[i].string();
  }
  return 0;
}
