/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "module_argument.h"
#include <getopt.h>
#include <unistd.h>
#include <iostream>

namespace myframe {

void ModuleArgument::DisplayUsage() {
  std::cout
    << "Usage: \n    " << binary_name_ << " [OPTION]...\n"
    << "Description: \n"
    << "    -h, --help : help infomation \n"
    << "    -c, --conf=${CONFIG_FILE} : module config file\n"
    << "    -d, --dir=${CONFIG_DIR} : module config dir\n"
    << "    -p, --process_name=${PROCESS_NAME}: "
        "The name of this launcher process, and it "
        "is also the name of log, default value is launcher_${PID}\n"
    << "Example:\n"
    << "    " << binary_name_ << " -h\n"
    << "    " << binary_name_ << " -c module1.conf -c module2.conf\n"
    << "    " << binary_name_ << " -c module1.conf -c module2.conf "
        "-p process_name\n"
    << "    " << binary_name_ << " -d service_dir -p process_name\n";
}

ModuleArgument::OptionReturnType ModuleArgument::ParseArgument(
  const int argc, char* const argv[]) {
  const std::string binary_name(argv[0]);
  binary_name_ = binary_name.substr(binary_name.find_last_of("/") + 1);
  process_name_ = binary_name_ + "_" + std::to_string(getpid());
  const std::string short_opts = "hc:d:p:";
  static const struct option long_opts[] = {
    {"help", no_argument, nullptr, 'h'},
    {"conf", required_argument, nullptr, 'c'},
    {"conf_dir", required_argument, nullptr, 'd'},
    {"process_name", required_argument, nullptr, 'p'},
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

}  // namespace myframe
