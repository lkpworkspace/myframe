/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>

namespace myframe {

void InitLog(const std::string& log_dir, const std::string& bin_name);

void ShutdownLog();

}  // namespace myframe
