/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <list>

namespace myframe {

class ModuleArgument final {
 public:
  enum OptionReturnType {
    kNoArgument,
    kGetHelpInfo,
    kInvalidArgument,
    kOtherParameter,
  };
  ModuleArgument() = default;
  ~ModuleArgument() = default;

  OptionReturnType ParseArgument(const int argc, char* const argv[]);
  void DisplayUsage();
  inline std::list<std::string> GetConfList() const { return conf_list_; }
  inline std::string GetConfDir() const { return conf_dir_; }
  inline std::string GetBinaryName() const { return binary_name_; }
  inline std::string GetProcessName() const { return process_name_; }
  inline std::string GetCmd() const { return cmd_; }
 private:
  std::string cmd_{""};
  std::string binary_name_{""};
  std::string process_name_{""};
  std::string conf_dir_{""};
  std::list<std::string> conf_list_;
};

}  // namespace myframe
