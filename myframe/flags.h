/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <gflags/gflags.h>

namespace myframe {

// params
DECLARE_uint32(myframe_worker_count);
DECLARE_uint32(myframe_dispatch_or_process_msg_max);
DECLARE_string(myframe_worker_timer_name);
DECLARE_uint32(myframe_conn_count);

// path
DECLARE_string(myframe_lib_dir);
DECLARE_string(myframe_log_dir);
DECLARE_string(myframe_service_dir);
DECLARE_string(myframe_conf_dir);

}  // namespace myframe
