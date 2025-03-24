/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <string_view>
#if __has_include(<filesystem>)
#include <filesystem>
namespace stdfs = std::filesystem;
#else
#error "no filesystem header"
#endif

#include <json/json.h>
#include "myframe/export.h"

namespace myframe {

class MYFRAME_EXPORT Common final {
 public:
  static std::vector<stdfs::path> GetDirFiles(const std::string& conf_path);
  static stdfs::path GetWorkRoot();
  static stdfs::path GetAbsolutePath(const std::string& flag_path);
  static bool IsAbsolutePath(const std::string& path);

  static Json::Value LoadJsonFromFile(const std::string& json_file);

  static std::vector<std::string_view> SplitMsgName(const std::string& name);

  static int SetThreadAffinity(std::thread* t, int cpu_core);
  static int SetThreadName(std::thread* t, const std::string& name);
  enum class SchedPriority : int {
    kLowest,
    kNormal,
    kRealtime,
  };
  static int SetProcessSchedPriority(SchedPriority);
  static int SetThreadSchedPriority(std::thread*, SchedPriority);
};

}  // namespace myframe
