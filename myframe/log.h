/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <filesystem>
#include "myframe/export.h"

namespace stdfs = std::filesystem;

namespace myframe {

MYFRAME_EXPORT void InitLog(
  const stdfs::path& log_dir,
  const std::string& bin_name);

MYFRAME_EXPORT void ShutdownLog();

}  // namespace myframe
