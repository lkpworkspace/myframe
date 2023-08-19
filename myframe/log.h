/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>

namespace myframe {

void InitLog(const std::string& log_dir, const std::string& bin_name);

void ShutdownLog();

}  // namespace myframe
