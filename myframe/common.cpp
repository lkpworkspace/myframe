/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/common.h"

#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

namespace myframe {

std::vector<std::string> Common::SplitMsgName(const std::string& name) {
  std::vector<std::string> name_list;
  std::string item;
  std::stringstream ss(name);
  while (std::getline(ss, item, '.')) {
    name_list.push_back(item);
  }
  return name_list;
}

std::vector<std::string> Common::GetDirFiles(const std::string& conf_path) {
  std::vector<std::string> res;
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

uint64_t Common::GetMonoTimeMs() {
  uint64_t t;
  struct timespec ti;
  clock_gettime(CLOCK_MONOTONIC, &ti);
  t = (uint64_t)ti.tv_sec * 1000;
  t += ti.tv_nsec / 1000000;
  return t;
}

bool Common::SetSockRecvTimeout(int fd, int timeout_ms) {
  struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
  return 0 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,
                         sizeof(timeout));
}

bool Common::SetNonblockFd(int fd, bool b) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (b) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }
  return fcntl(fd, F_SETFL, flags) != -1;
}

bool Common::IsBlockFd(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  return !(flags & O_NONBLOCK);
}

stdfs::path Common::GetWorkRoot() {
  char path_buf[256];
  memset(path_buf, 0, sizeof(path_buf));
  int ret = readlink("/proc/self/exe", path_buf, sizeof(path_buf));
  if (ret == -1) {
    return "";
  }
  if (ret >= sizeof(path_buf)) {
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

}  // namespace myframe
