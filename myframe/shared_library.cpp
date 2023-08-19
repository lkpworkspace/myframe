/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/shared_library.h"

#include <dlfcn.h>

#include <glog/logging.h>

namespace myframe {

SharedLibrary::~SharedLibrary() {
  Unload();
}

bool SharedLibrary::Load(const std::string& path) {
  return Load(path, Flags::kGlobal);
}

bool SharedLibrary::Load(
    const std::string& path,
    Flags flags) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ != nullptr) {
    return false;
  }
  int real_flag = RTLD_NOW;
  if (static_cast<int>(flags) & static_cast<int>(Flags::kLocal)) {
    real_flag |= RTLD_LOCAL;
  } else {
    real_flag |= RTLD_GLOBAL;
  }
  handle_ = dlopen(path.c_str(), real_flag);
  if (handle_ == nullptr) {
    LOG(ERROR) << "Open dll " << path << " failed, " << dlerror();
    return false;
  }
  path_ = path;
  return true;
}

void SharedLibrary::Unload() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return;
  }
  if (dlclose(handle_)) {
    LOG(ERROR) << "lib " << path_ << " close failed, " << dlerror();
  }
  handle_ = nullptr;
}

bool SharedLibrary::IsLoaded() {
  std::lock_guard<std::mutex> lock(mutex_);
  return handle_ != nullptr;
}

bool SharedLibrary::HasSymbol(const std::string& name) {
  return GetSymbol(name) != nullptr;
}

void* SharedLibrary::GetSymbol(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return nullptr;
  }

  void* result = dlsym(handle_, name.c_str());
  if (result == nullptr) {
    LOG(ERROR) << "lib " << path_
      << " has no symbol " << name << ", " << dlerror();
    return nullptr;
  }
  return result;
}

}  // namespace myframe
