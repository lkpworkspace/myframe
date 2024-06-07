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
  static Json::Value LoadJsonFromFile(const std::string& json_file);

  static stdfs::path GetWorkRoot();
  static stdfs::path GetAbsolutePath(const std::string& flag_path);
  static bool IsAbsolutePath(const std::string& path);

  static std::vector<std::string> SplitMsgName(const std::string& name);
};

}  // namespace myframe
