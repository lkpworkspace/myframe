/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <string>
#include <memory>

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
  virtual ~SharedLibrary() = default;

  static std::shared_ptr<SharedLibrary> Create();

  virtual bool Load(const std::string& path) = 0;
  virtual bool Load(const std::string& path, Flags flags) = 0;

  virtual void Unload() = 0;

  virtual bool IsLoaded() = 0;

  virtual bool HasSymbol(const std::string& name) = 0;

  virtual void* GetSymbol(const std::string& name) = 0;

  inline const std::string& GetPath() const { return path_; }

 protected:
  inline void SetPath(const std::string& path) { path_ = path; }

 private:
  std::string path_;

  DISALLOW_COPY_AND_ASSIGN(SharedLibrary)
};

}  // namespace myframe
