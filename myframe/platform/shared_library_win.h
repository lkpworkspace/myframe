/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>

#include <windows.h>
#include <glog/logging.h>

#include "myframe/macros.h"
#include "myframe/shared_library.h"

namespace myframe {

class SharedLibraryWin final : public SharedLibrary {
 public:
  SharedLibraryWin() = default;
  virtual ~SharedLibraryWin();

  bool Load(const std::string& path) override;
  bool Load(const std::string& path, Flags flags) override;

  void Unload() override;

  bool IsLoaded() override;

  bool HasSymbol(const std::string& name) override;

  void* GetSymbol(const std::string& name) override;

 private:
  HMODULE handle_{ nullptr };
  std::mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(SharedLibraryWin)
};

SharedLibraryWin::~SharedLibraryWin() {
  Unload();
}

bool SharedLibraryWin::Load(const std::string& path) {
  return Load(path, Flags::kGlobal);
}

bool SharedLibraryWin::Load(
    const std::string& path,
    Flags flags) {
  (void)flags;
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ != nullptr) {
    return false;
  }
  handle_ = LoadLibrary(path.c_str());
  if (handle_ == nullptr) {
    LOG(ERROR) << "Open dll " << path << " failed";
    FreeLibrary(handle_);
    return false;
  }
  SetPath(path);
  return true;
}

void SharedLibraryWin::Unload() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return;
  }
  if (!FreeLibrary(handle_)) {
    LOG(ERROR) << "lib " << GetPath() << " close failed";
  }
  handle_ = nullptr;
}

bool SharedLibraryWin::IsLoaded() {
  std::lock_guard<std::mutex> lock(mutex_);
  return handle_ != nullptr;
}

bool SharedLibraryWin::HasSymbol(const std::string& name) {
  return GetSymbol(name) != nullptr;
}

void* SharedLibraryWin::GetSymbol(const std::string& name) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (handle_ == nullptr) {
    return nullptr;
  }

  void* result = GetProcAddress(handle_, name.c_str());
  if (result == nullptr) {
    LOG(ERROR) << "lib " << GetPath()
      << " has no symbol " << name;
    return nullptr;
  }
  return result;
}

}  // namespace myframe
