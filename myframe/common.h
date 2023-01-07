/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <jsoncpp/json/json.h>

namespace myframe {

template <typename T>
void ListAppend(std::list<std::shared_ptr<T>>* dst,
                std::list<std::shared_ptr<T>>* src) {
  dst->insert(dst->end(), src->begin(), src->end());
  src->clear();
}

std::vector<std::string> SplitMsgName(const std::string& name);

class Common {
 public:
  static std::vector<std::string> GetDirFiles(const std::string& conf_path);
  static Json::Value LoadJsonFromFile(const std::string& json_file);
  static uint64_t GetMonoTimeMs();
  static bool SetSockRecvTimeout(int fd, int timeout_ms);
  static bool SetNonblockFd(int fd, bool b);
  static bool IsBlockFd(int fd);
};

}  // namespace myframe
