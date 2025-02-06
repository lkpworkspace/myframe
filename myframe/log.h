/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>

#include <glog/logging.h>

#include "myframe/export.h"
#include "myframe/common.h"

namespace myframe {

MYFRAME_EXPORT void InitLog(
  const stdfs::path& log_dir,
  const std::string& bin_name,
  int max_size_mb = 100);

MYFRAME_EXPORT void ShutdownLog();

}  // namespace myframe
