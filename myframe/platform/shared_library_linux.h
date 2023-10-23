/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <dlfcn.h>

#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>

#include <glog/logging.h>

#include "myframe/macros.h"
#include "myframe/shared_library.h"

namespace myframe {

class SharedLibraryLinux final : public SharedLibrary {
 public:
  SharedLibraryLinux() = default;
  virtual ~SharedLibraryLinux();

  bool Load(const std::string& path) override;
  bool Load(const std::string& path, Flags flags) override;

  void Unload() override;

  bool IsLoaded() override;

  bool HasSymbol(const std::string& name) override;

  void* GetSymbol(const std::string& name) override;

 private:
  void* handle_{ nullptr };
  std::mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(SharedLibraryLinux)
};

SharedLibraryLinux::~SharedLibraryLinux() {
  Unload();
}

bool SharedLibraryLinux::Load(const std::string& path) {
  return Load(path, Flags::kGlobal);
}

bool SharedLibraryLinux::Load(
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
  SetPath(path);
  return true;
}

void SharedLibraryLinux::Unload() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return;
  }
  if (dlclose(handle_)) {
    LOG(ERROR) << "lib " << GetPath() << " close failed, " << dlerror();
  }
  handle_ = nullptr;
}

bool SharedLibraryLinux::IsLoaded() {
  std::lock_guard<std::mutex> lock(mutex_);
  return handle_ != nullptr;
}

bool SharedLibraryLinux::HasSymbol(const std::string& name) {
  return GetSymbol(name) != nullptr;
}

void* SharedLibraryLinux::GetSymbol(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return nullptr;
  }

  void* result = dlsym(handle_, name.c_str());
  if (result == nullptr) {
    LOG(ERROR) << "lib " << GetPath()
      << " has no symbol " << name << ", " << dlerror();
    return nullptr;
  }
  return result;
}

}  // namespace myframe
