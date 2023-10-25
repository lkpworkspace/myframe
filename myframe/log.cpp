/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/log.h"

#include <string>
#include <iostream>

#include <glog/logging.h>

static void signal_handler(const char *data, size_t size) {
  std::string str = std::string(data, size);
  std::cerr << str;
  LOG(ERROR) << "\n" << str;
}

namespace myframe {

void InitLog(const stdfs::path& log_dir, const std::string& bin_name) {
  google::InitGoogleLogging(bin_name.c_str());

  FLAGS_logbufsecs = 0;
  FLAGS_max_log_size = 100;
  FLAGS_stop_logging_if_full_disk = true;

  std::string dst_str = (log_dir / bin_name).string();
  google::SetLogDestination(google::GLOG_ERROR, "");
  google::SetLogDestination(google::GLOG_WARNING, "");
  google::SetLogDestination(google::GLOG_FATAL, "");
  google::SetLogDestination(google::GLOG_INFO, dst_str.c_str());

  google::InstallFailureSignalHandler();
  google::InstallFailureWriter(&signal_handler);
}

void ShutdownLog() {
  google::ShutdownGoogleLogging();
}

}  // namespace myframe
