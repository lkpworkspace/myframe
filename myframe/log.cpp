/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/log.h"

#include <string>
#include <iostream>

#include <glog/logging.h>

#include "myframe/flags.h"
#include "myframe/common.h"

static void signal_handler(const char *data, int size) {
  std::string str = std::string(data, size);
  std::cerr << str;
  LOG(ERROR) << "\n" << str;
}

namespace myframe {

void InitLog(const char* bin_name) {
  if (!google::IsGoogleLoggingInitialized()) {
    google::InitGoogleLogging(bin_name);
  }

  FLAGS_logbufsecs = 0;
  FLAGS_max_log_size = 100;
  FLAGS_stop_logging_if_full_disk = true;

  auto log_dir = Common::GetAbsolutePath(FLAGS_myframe_log_dir);
  std::string dst_str = log_dir + bin_name;
  google::SetLogDestination(google::ERROR, "");
  google::SetLogDestination(google::WARNING, "");
  google::SetLogDestination(google::FATAL, "");
  google::SetLogDestination(google::INFO, dst_str.c_str());

  google::InstallFailureSignalHandler();
  google::InstallFailureWriter(&signal_handler);
}

}  // namespace myframe
