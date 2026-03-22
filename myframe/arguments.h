/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include <list>
#include <vector>

#include "myframe/export.h"

namespace myframe {

class MYFRAME_EXPORT Argument {
 public:
  enum class ArgType : std::uint8_t {
    kArgString = 0,
    kArgInteger,
  };
  ArgType type;
  std::string key;
  int value_int;
  std::string value_str;

  std::string DebugString();
};

class MYFRAME_EXPORT Arguments : public std::vector<Argument> {
 public:
  void SetString(const std::string& key, const std::string& value);

  void SetInt(const std::string& key, int value);

  std::string DebugString();
};

#define MYFRAME_SERVICE_LIB_DIR "app.lib_dir"

#define MYFRAME_THREAD_POOL_SIZE "app.thread_pool_size"

#define MYFRAME_WARNING_MSG_SIZE "app.warning_msg_size"

#define MYFRAME_PENDING_QUEUE_SIZE "app.pending_queue_size"

#define MYFRAME_RUN_QUEUE_SIZE "app.run_queue_size"

#define MYFRAME_EVENT_CONNE_SIZE "app.event_conn_size"

}  // namespace myframe
