/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <sstream>
#include "myframe/arguments.h"

namespace myframe {

std::string Argument::DebugString() {
  std::stringstream ss;
  ss << key << "(";
  if (type == Argument::ArgType::kArgInteger) {
    ss << "int):";
    ss << value_int;
  }
  if (type == Argument::ArgType::kArgString) {
    ss << "string):";
    ss << value_str;
  }
  return ss.str();
}

void Arguments::SetString(const std::string& key, const std::string& value) {
  Argument arg;
  arg.type = Argument::ArgType::kArgString;
  arg.key = key;
  arg.value_str = value;
  push_back(arg);
}

void Arguments::SetInt(const std::string& key, int value) {
  Argument arg;
  arg.type = Argument::ArgType::kArgInteger;
  arg.key = key;
  arg.value_int = value;
  push_back(arg);
}

std::string Arguments::DebugString() {
  std::stringstream ss;
  for (auto it = begin(); it != end(); ++it) {
    ss << it->DebugString() << std::endl;
  }
  return ss.str();
}

}  // namespace myframe
