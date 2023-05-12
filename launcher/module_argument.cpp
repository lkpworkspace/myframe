/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "module_argument.h"
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include "myframe/common.h"

namespace myframe {

ModuleArgument::ModuleArgument(
    const std::string& sys_conf_dir) {
  sys_conf_dir_ = myframe::Common::GetAbsolutePath(sys_conf_dir);
}

void ModuleArgument::DisplayUsage() {
  std::cout
    << "Usage: \n    " << binary_name_ << " [OPTION]...\n"
    << "Description: \n"
    << "    -h, --help : help infomation \n"
    << "    -s, --sys_conf=${CONFIG_FILE} : framework config file \n"
    << "    -c, --conf=${CONFIG_FILE} : module config file\n"
    << "    -d, --dir=${CONFIG_DIR} : module config dir\n"
    << "    -p, --process_name=${PROCESS_NAME}: "
        "The name of this launcher process, and it "
        "is also the name of log, default value is launcher_${PID}\n"
    << "Example:\n"
    << "    " << binary_name_ << " -h\n"
    << "    " << binary_name_ << " -c module1.json -c module2.json\n"
    << "    " << binary_name_ << " -c module1.json -c module2.json "
        "-p process_name\n"
    << "    " << binary_name_ << " -s sys.json -c module1.json "
        "-p process_name\n"
    << "    " << binary_name_ << " -d service_dir -p process_name\n";
}

ModuleArgument::OptionReturnType ModuleArgument::ParseArgument(
  const int argc, char* const argv[]) {
  const std::string binary_name(argv[0]);
  binary_name_ = binary_name.substr(binary_name.find_last_of("/") + 1);
  process_name_ = binary_name_ + "_" + std::to_string(getpid());
  const std::string short_opts = "hc:d:p:s:";
  static const struct option long_opts[] = {
    {"help", no_argument, nullptr, 'h'},
    {"conf", required_argument, nullptr, 'c'},
    {"conf_dir", required_argument, nullptr, 'd'},
    {"process_name", required_argument, nullptr, 'p'},
    {"sys_conf", required_argument, nullptr, 's'},
    {NULL, no_argument, nullptr, 0}};

  // log command for info
  std::string cmd("");
  for (int i = 0; i < argc; ++i) {
    cmd += argv[i];
    cmd += " ";
  }
  cmd_ = cmd;

  OptionReturnType ret = kNoArgument;
  bool parsing_next_opt = true;
  int long_index = 0;
  do {
    int opt = getopt_long(
      argc, argv, short_opts.c_str(), long_opts, &long_index);
    if (opt == -1) {
      break;
    }
    switch (opt) {
      case 's':
        if (!ParseSysConf(optarg)) {
          parsing_next_opt = false;
          ret = kInvalidArgument;
        }
        break;
      case 'c':
        conf_list_.emplace_back(optarg);
        ret = kOtherParameter;
        break;
      case 'd':
        conf_dir_ = optarg;
        ret = kOtherParameter;
        break;
      case 'p':
        process_name_ = optarg;
        ret = kOtherParameter;
        break;
      case 'h':
        parsing_next_opt = false;
        ret = kGetHelpInfo;
        break;
      default:
        ret = kInvalidArgument;
        break;
    }
  } while (parsing_next_opt);

  return ret;
}

bool ModuleArgument::ParseSysConf(const std::string& sys_conf) {
  std::string full_sys_conf;
  if (Common::IsAbsolutePath(sys_conf)) {
    full_sys_conf = sys_conf;
  } else {
    full_sys_conf = sys_conf_dir_ + sys_conf;
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
  return true;
}

}  // namespace myframe
