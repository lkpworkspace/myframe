/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>

#include "myframe/macros.h"

namespace myframe {

class SharedLibrary {
 public:
  enum class Flags : int {
    // On platforms that use dlopen(), use RTLD_GLOBAL. This is the default
    // if no flags are given.
    kGlobal = 1,

    // On platforms that use dlopen(), use RTLD_LOCAL instead of RTLD_GLOBAL.
    //
    // Note that if this flag is specified, RTTI (including dynamic_cast and
    // throw) will not work for types defined in the shared library with GCC
    // and possibly other compilers as well. See
    // http://gcc.gnu.org/faq.html#dso for more information.
    kLocal = 2,
  };

  SharedLibrary() = default;
  virtual ~SharedLibrary();

  bool Load(const std::string& path);
  bool Load(const std::string& path, Flags flags);

  void Unload();

  bool IsLoaded();

  bool HasSymbol(const std::string& name);

  void* GetSymbol(const std::string& name);

  inline const std::string& GetPath() const { return path_; }

 private:
  void* handle_{ nullptr };
  std::string path_;
  std::mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(SharedLibrary)
};

}  // namespace myframe
