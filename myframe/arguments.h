/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include <list>
#include <vector>

#include "myframe/export.h"

namespace myframe {

class MYFRAME_EXPORT Argument {
 public:
  enum class ArgType : std::uint8_t {
    kArgString = 0,
    kArgInteger,
  };
  ArgType type;
  std::string key;
  int value_int;
  std::string value_str;

  std::string DebugString();
};

class MYFRAME_EXPORT Arguments : public std::vector<Argument> {
 public:
  void SetStr(const std::string& key, const std::string& value);

  void SetInt(const std::string& key, int value);

  std::string DebugString();
};

}  // namespace myframe
