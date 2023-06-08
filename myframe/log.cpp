/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/log.h"

#include <string>
#include <iostream>

#include <glog/logging.h>

#include "myframe/common.h"

static void signal_handler(const char *data, int size) {
  std::string str = std::string(data, size);
  std::cerr << str;
  LOG(ERROR) << "\n" << str;
}

namespace myframe {

void InitLog(const std::string& log_dir, const std::string& bin_name) {
  google::InitGoogleLogging(bin_name.c_str());

  FLAGS_logbufsecs = 0;
  FLAGS_max_log_size = 100;
  FLAGS_stop_logging_if_full_disk = true;

  auto full_log_dir = Common::GetAbsolutePath(log_dir);
  std::string dst_str = full_log_dir + bin_name;
  google::SetLogDestination(google::ERROR, "");
  google::SetLogDestination(google::WARNING, "");
  google::SetLogDestination(google::FATAL, "");
  google::SetLogDestination(google::INFO, dst_str.c_str());

  google::InstallFailureSignalHandler();
  google::InstallFailureWriter(&signal_handler);
}

void ShutdownLog() {
  google::ShutdownGoogleLogging();
}

}  // namespace myframe
