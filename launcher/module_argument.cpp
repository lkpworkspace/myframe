/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "module_argument.h"
#include "myframe/platform.h"
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
#include <unistd.h>
#elif defined(MYFRAME_OS_WINDOWS)
#include <process.h>
#endif
#include <iostream>

namespace myframe {

ModuleArgument::ModuleArgument(
    const std::string& default_sys_conf_dir) {
  default_sys_conf_dir_ = myframe::Common::GetAbsolutePath(
    default_sys_conf_dir);
  parser_.add<std::string>("process_name", 'p',
    "The name of this launcher process, "
    "and it is also the name of log, "
    "default value is launcher_${PID}\n",
    false, "");
  parser_.add<std::string>("sys_conf", 's',
    "framework config file",
    false, "");
  parser_.add<std::string>("dir", 'd',
    "module config dir",
    false, "");
  parser_.add<std::string>("log_dir", 0,
    "framework log dir",
    false, "");
  parser_.add<std::string>("lib_dir", 0,
    "framework lib dir",
    false, "");
  parser_.footer("module_config_file ...");
}

void ModuleArgument::ParseArgument(
  const int argc, char** argv) {
  binary_name_ = stdfs::path(argv[0]).filename().string();
  process_name_ = binary_name_ + "_" + std::to_string(getpid());

  // log command for info
  std::string cmd("");
  for (int i = 0; i < argc; ++i) {
    cmd += argv[i];
    cmd += " ";
  }
  cmd_ = cmd;

  // check args
  parser_.parse_check(argc, argv);

  auto process_name = parser_.get<std::string>("process_name");
  if (!process_name.empty()) {
    process_name_ = process_name;
  }

  auto sys_conf = parser_.get<std::string>("sys_conf");
  if (!sys_conf.empty()) {
    if (!ParseSysConf(sys_conf)) {
      std::cerr << "parse sys conf failed!!" << std::endl;
      exit(-1);
    }
  }

  auto dir = parser_.get<std::string>("dir");
  if (!dir.empty()) {
    conf_dir_ = dir;
  }

  for (size_t i = 0; i < parser_.rest().size(); i++) {
    conf_list_.emplace_back(parser_.rest()[i]);
  }

  auto log_dir = parser_.get<std::string>("log_dir");
  if (!log_dir.empty()) {
    log_dir_ = log_dir;
  }

  auto lib_dir = parser_.get<std::string>("lib_dir");
  if (!lib_dir.empty()) {
    lib_dir_ = lib_dir;
  }
}

bool ModuleArgument::ParseSysConf(const std::string& sys_conf) {
  std::string full_sys_conf;
  if (Common::IsAbsolutePath(sys_conf)) {
    full_sys_conf = sys_conf;
  } else {
    full_sys_conf = (default_sys_conf_dir_ / sys_conf).string();
  }
  auto root = Common::LoadJsonFromFile(full_sys_conf);
  if (root.isNull()
      || !root.isObject()) {
    return false;
  }
  if (root.isMember("thread_poll_size")
      && root["thread_poll_size"].isInt()
      && root["thread_poll_size"].asInt() > 0
      && root["thread_poll_size"].asInt() < 1024) {
    thread_poll_size_ = root["thread_poll_size"].asInt();
  }
  if (root.isMember("conn_event_size")
      && root["conn_event_size"].isInt()
      && root["conn_event_size"].asInt() > 0
      && root["conn_event_size"].asInt() < 1024) {
    conn_event_size_ = root["conn_event_size"].asInt();
  }
  if (root.isMember("warning_msg_size")
      && root["warning_msg_size"].isInt()
      && root["warning_msg_size"].asInt() > 0
      && root["warning_msg_size"].asInt() < 1024) {
    warning_msg_size_ = root["warning_msg_size"].asInt();
  }
  if (root.isMember("log_dir")
      && root["log_dir"].isString()) {
    log_dir_ = root["log_dir"].asString();
  }
  if (root.isMember("lib_dir")
      && root["lib_dir"].isString()) {
    lib_dir_ = root["lib_dir"].asString();
  }
  if (root.isMember("service_dir")
      && root["service_dir"].isString()) {
    conf_dir_ = root["service_dir"].asString();
  }
  return true;
}

}  // namespace myframe
