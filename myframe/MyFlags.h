#pragma once
#include <gflags/gflags.h>

// params
DECLARE_uint32(worker_count);
DECLARE_uint32(dispatch_or_process_msg_max);

// path
DECLARE_string(myframe_lib_dir);
DECLARE_string(myframe_log_dir);
DECLARE_string(myframe_service_conf_dir);