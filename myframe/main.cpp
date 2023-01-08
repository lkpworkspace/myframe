/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <gflags/gflags.h>

#include "myframe/app.h"
#include "myframe/common.h"
#include "myframe/flags.h"
#include "myframe/log.h"

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  // 利用构造函数初始化日志系统
  myframe::Log log;

  // 初始化并启动线程
  auto app = std::make_shared<myframe::App>();
  if (false == app->Init()) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // 从配置文件加载服务
  auto service_dir =
      myframe::Common::GetAbsolutePath(myframe::FLAGS_myframe_service_dir);
  if (!app->LoadModsFromConf(service_dir)) {
    LOG(ERROR) << "Load service failed, exit";
    return -1;
  }

  // 开始事件循环
  return app->Exec();
}
