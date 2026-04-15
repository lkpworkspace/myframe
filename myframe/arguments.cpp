/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <sstream>
#include <json/json.h>
#include "myframe/arguments.h"
#include "myframe/common.h"
#include "myframe/log.h"

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

/*
{
  "key":"value",
  ...
}
*/
bool Arguments::Load(const std::string& filepath) {
  // 加载配置文件
  Json::Value root = Common::LoadJsonFromFile(filepath);
  if (root.isNull()) {
    return false;
  }
  Json::Value::Members arg_list = root.getMemberNames();
  for (auto arg_key = arg_list.begin(); arg_key != arg_list.end(); ++arg_key) {
    const auto& arg_value = root[*arg_key];
    if (arg_value.isInt()) {
      Argument arg;
      arg.type = Argument::ArgType::kArgInteger;
      arg.key = *arg_key;
      arg.value_int = arg_value.asInt();
      push_back(arg);
    } else if (arg_value.isString()) {
      Argument arg;
      arg.type = Argument::ArgType::kArgString;
      arg.key = *arg_key;
      arg.value_str = arg_value.asString();
      push_back(arg);
    } else {
      LOG(WARNING) << "Unknown Arg \"" << *arg_key << "\":"
        << arg_value.toStyledString();
    }
  }
  return true;
}

bool Arguments::Save(const std::string& filepath) {
  // 保存配置文件
  Json::Value root;
  for (auto it = begin(); it != end(); ++it) {
    auto type = it->type;
    if (type == Argument::ArgType::kArgInteger) {
      root[it->key] = it->value_int;
    }
    if (type == Argument::ArgType::kArgString) {
      root[it->key] = it->value_str;
    }
  }
  return Common::SaveFile(filepath, root.toStyledString());
}

void Arguments::SetStr(const std::string& key, const std::string& value) {
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
