/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/log.h"

#include <string>

#include "myframe/flags.h"
#include "myframe/common.h"

static void signal_handler(const char *data, int size) {
  std::string str = std::string(data, size);
  LOG(ERROR) << str;
}

namespace myframe {

Log::Log() {
  // output log immediately
  FLAGS_logbufsecs = 0;
  // set the log file to 100MB
  FLAGS_max_log_size = 100;
  FLAGS_stop_logging_if_full_disk = true;
  // install core handle
  google::InstallFailureSignalHandler();
  google::InstallFailureWriter(&signal_handler);
  // init glog
  google::InitGoogleLogging("myframe");
  // log with level >=ERROR is output to stderr
  google::SetStderrLogging(google::GLOG_FATAL);
  // set the path for the log file
  auto log_dir = Common::GetAbsolutePath(FLAGS_myframe_log_dir);
  std::string dest_dir = log_dir + "info";
  google::SetLogDestination(google::GLOG_INFO, dest_dir.c_str());
}

}  // namespace myframe
