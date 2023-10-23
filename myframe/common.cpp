/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/common.h"

#include <string.h>

#include "myframe/platform.h"
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
#include <unistd.h>
#elif defined(MYFRAME_OS_WINDOWS)
#include <Windows.h>
#else
#error "Platform not supported"
#endif

#include <fstream>
#include <sstream>

namespace myframe {

std::vector<stdfs::path> Common::GetDirFiles(const std::string& conf_path) {
  std::vector<stdfs::path> res;
  stdfs::path path(conf_path);
  for (auto const& dir_entry : stdfs::directory_iterator{path}) {
    if (dir_entry.is_regular_file()) {
      res.emplace_back(dir_entry.path());
    }
  }
  return res;
}

Json::Value Common::LoadJsonFromFile(const std::string& json_file) {
  std::ifstream ifs(json_file);
  if (!ifs.is_open()) {
    return Json::Value::nullSingleton();
  }
  Json::Value root;
  Json::Reader reader(Json::Features::strictMode());
  if (!reader.parse(ifs, root)) {
    ifs.close();
    return Json::Value::nullSingleton();
  }
  ifs.close();
  return root;
}

stdfs::path Common::GetWorkRoot() {
  char path_buf[256];
  memset(path_buf, 0, sizeof(path_buf));
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  int ret = readlink("/proc/self/exe", path_buf, sizeof(path_buf));
  if (ret == -1) {
    return "";
  }
#elif defined(MYFRAME_OS_WINDOWS)
  auto ret = GetModuleFileName(NULL, path_buf, sizeof(path_buf));
  if (ret == 0) {
    return "";
  }
#else
  #error "Platform not supported"
#endif
  if (static_cast<std::size_t>(ret) >= sizeof(path_buf)) {
    path_buf[sizeof(path_buf) - 1] = '\0';
  }
  stdfs::path p(path_buf);
  if (p.has_parent_path()) {
    p = p.parent_path();
    if (p.has_parent_path()) {
      p = p.parent_path();
    }
  }
  return p;
}

stdfs::path Common::GetAbsolutePath(const std::string& flag_path) {
  stdfs::path p(flag_path);
  if (p.is_absolute()) {
    return flag_path;
  }
  auto root = GetWorkRoot();
  if (root.empty()) {
    return flag_path;
  }
  root /= p;
  return root;
}

bool Common::IsAbsolutePath(const std::string& path) {
  stdfs::path p(path);
  if (p.is_absolute()) {
    return true;
  }
  return false;
}

std::vector<std::string> Common::SplitMsgName(const std::string& name) {
  std::vector<std::string> name_list;
  std::string item;
  std::stringstream ss(name);
  while (std::getline(ss, item, '.')) {
    name_list.push_back(item);
  }
  return name_list;
}

}  // namespace myframe
