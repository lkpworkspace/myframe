/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/common.h"

#include <string.h>

#include "myframe/platform.h"
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
#include <dirent.h>
#include <unistd.h>
#else
#error "Platform not supported"
#endif

#include <fstream>
#include <sstream>


namespace myframe {

std::vector<std::string> Common::GetDirFiles(const std::string& conf_path) {
  std::vector<std::string> res;
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  DIR* dir = opendir(conf_path.c_str());
  if (dir == nullptr) {
    return res;
  }
  struct dirent* entry = nullptr;
  while (nullptr != (entry = readdir(dir))) {
    if (entry->d_type == DT_REG) {
      res.emplace_back(conf_path + entry->d_name);
    }
  }
  closedir(dir);
#else
  #error "Platform not supported"
#endif
  return res;
}

Json::Value Common::LoadJsonFromFile(const std::string& json_file) {
  std::ifstream ifs(json_file);
  if (!ifs.is_open()) {
    return Json::Value::null;
  }
  Json::Value root;
  Json::Reader reader(Json::Features::strictMode());
  if (!reader.parse(ifs, root)) {
    ifs.close();
    return Json::Value::null;
  }
  ifs.close();
  return root;
}

stdfs::path Common::GetWorkRoot() {
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  char path_buf[256];
  memset(path_buf, 0, sizeof(path_buf));
  int ret = readlink("/proc/self/exe", path_buf, sizeof(path_buf));
  if (ret == -1) {
    return "";
  }
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
#else
  #error "Platform not supported"
#endif
}

std::string Common::GetAbsolutePath(const std::string& flag_path) {
  stdfs::path p(flag_path);
  if (p.is_absolute()) {
    return flag_path;
  }
  p += "/";
  auto root = GetWorkRoot();
  if (root.empty()) {
    return flag_path;
  }
  root += "/";
  root += p;
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
