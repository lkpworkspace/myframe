/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <csignal>
#include <iostream>

#include "myframe/log.h"
#include "myframe/platform.h"
#include "myframe/common.h"
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

  // 初始化日志和参数
  auto root_dir = myframe::Common::GetWorkRoot();
  stdfs::path log_dir;
  stdfs::path lib_dir;
  stdfs::path service_dir;
  stdfs::path conf_dir;

  if (module_args.GetLogDir().empty()) {
    log_dir = MYFRAME_LOG_DIR;
  } else {
    log_dir = module_args.GetLogDir();
  }
  log_dir = myframe::Common::GetAbsolutePath(log_dir.string());
  myframe::InitLog(log_dir, module_args.GetProcessName());
  LOG(INFO) << "launch command: " << module_args.GetCmd();

  if (module_args.GetLibDir().empty()) {
  #if defined(MYFRAME_OS_WINDOWS)
    lib_dir = MYFRAME_BIN_DIR;
  #else
    lib_dir = MYFRAME_LIB_DIR;
  #endif
  } else {
    lib_dir = module_args.GetLibDir();
  }
  lib_dir = myframe::Common::GetAbsolutePath(lib_dir.string());

  if (module_args.GetConfDir().empty()) {
    service_dir = MYFRAME_SERVICE_DIR;
  } else {
    service_dir = module_args.GetConfDir();
  }
  service_dir = myframe::Common::GetAbsolutePath(service_dir.string());

  conf_dir = myframe::Common::GetAbsolutePath(MYFRAME_CONF_DIR);

  LOG(INFO) << "root dir: " << root_dir.string();
  LOG(INFO) << "default lib dir: " << lib_dir.string();
  LOG(INFO) << "default service dir: " << service_dir.string();
  LOG(INFO) << "default log dir: " << log_dir.string();
  LOG(INFO) << "default conf dir: " << conf_dir.string();

  // 初始化并启动线程
  g_app = std::make_shared<myframe::App>();
  if (false == g_app->Init(
    lib_dir.string(),
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
        abs_conf_file = (service_dir / conf).string();
      }
      if (!g_app->LoadServiceFromFile(abs_conf_file)) {
        LOG(ERROR) << "Load " << abs_conf_file << " failed, exit";
        g_app->Quit();
        break;
      }
    }
  } else {
    if (g_app->LoadServiceFromDir(service_dir.string()) <= 0) {
      LOG(ERROR) << "Load service from " << service_dir.string()
        << " failed, exit";
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
