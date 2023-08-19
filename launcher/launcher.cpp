/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <csignal>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/log.h"
#include "myframe/app.h"
#include "launcher_config.h"
#include "module_argument.h"

static std::shared_ptr<myframe::App> g_app{nullptr};

void OnShutDown(int sig) {
  if (g_app == nullptr) {
    return;
  }
  LOG(INFO) << "received interrupt " << sig << ", quiting...";
  g_app->Quit();
}

int main(int argc, char** argv) {
  // 命令行参数解析
  myframe::ModuleArgument module_args(MYFRAME_CONF_DIR);
  module_args.ParseArgument(argc, argv);

  // 初始化日志系统
  myframe::InitLog(MYFRAME_LOG_DIR, module_args.GetProcessName());
  LOG(INFO) << "launch command: " << module_args.GetCmd();
  std::string root_dir = myframe::Common::GetWorkRoot();
  auto lib_dir = myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR);
  auto service_dir = myframe::Common::GetAbsolutePath(MYFRAME_SERVICE_DIR);
  auto log_dir = myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR);
  auto conf_dir = myframe::Common::GetAbsolutePath(MYFRAME_CONF_DIR);
  LOG(INFO) << "root dir: " << root_dir;
  LOG(INFO) << "default lib dir: " << lib_dir;
  LOG(INFO) << "default service dir: " << service_dir;
  LOG(INFO) << "default log dir: " << log_dir;
  LOG(INFO) << "default conf dir: " << conf_dir;

  // 初始化并启动线程
  g_app = std::make_shared<myframe::App>();
  if (false == g_app->Init(
    lib_dir,
    module_args.GetThreadPoolSize(),
    module_args.GetConnEventSize(),
    module_args.GetWarningMsgSize())) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // 从配置文件加载服务
  if (!module_args.GetConfList().empty()) {
    auto conf_list = module_args.GetConfList();
    for (auto conf : conf_list) {
      std::string abs_conf_file;
      if (myframe::Common::IsAbsolutePath(conf)) {
        abs_conf_file = conf;
      } else {
        abs_conf_file = service_dir + conf;
      }
      if (!g_app->LoadServiceFromFile(abs_conf_file)) {
        LOG(ERROR) << "Load " << abs_conf_file << " failed, exit";
        g_app->Quit();
        break;
      }
    }
  } else {
    std::string abs_service_dir;
    if (!module_args.GetConfDir().empty()) {
       if (myframe::Common::IsAbsolutePath(module_args.GetConfDir())) {
        abs_service_dir = module_args.GetConfDir();
      } else {
        abs_service_dir = root_dir + "/" + module_args.GetConfDir() + "/";
      }
    } else {
      abs_service_dir = service_dir;
    }
    if (g_app->LoadServiceFromDir(abs_service_dir) <= 0) {
      LOG(ERROR) << "Load service from " << abs_service_dir << " failed, exit";
      g_app->Quit();
    }
  }

  // 注册退出函数
  std::signal(SIGINT, OnShutDown);

  // 开始事件循环
  g_app->Exec();

  // 退出资源清理
  g_app = nullptr;
  LOG(INFO) << "launcher exit";
  myframe::ShutdownLog();
  return 0;
}
