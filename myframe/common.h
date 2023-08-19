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
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#else
#error "no filesystem"
#endif

#include <jsoncpp/json/json.h>
#include "myframe/export.h"

namespace myframe {

class MYFRAME_EXPORT Common final {
 public:
  static std::vector<std::string> GetDirFiles(const std::string& conf_path);
  static Json::Value LoadJsonFromFile(const std::string& json_file);
  static uint64_t GetMonoTimeMs();
  static bool SetSockRecvTimeout(int fd, int timeout_ms);
  static bool SetNonblockFd(int fd, bool b);
  static bool IsBlockFd(int fd);
  static stdfs::path GetWorkRoot();
  static std::string GetAbsolutePath(const std::string& flag_path);
  static bool IsAbsolutePath(const std::string& path);
  template <typename T>
  static void ListAppend(
    std::list<std::shared_ptr<T>>* dst,
    std::list<std::shared_ptr<T>>* src) {
    dst->insert(dst->end(), src->begin(), src->end());
    src->clear();
  }
  static std::vector<std::string> SplitMsgName(const std::string& name);
};

}  // namespace myframe
