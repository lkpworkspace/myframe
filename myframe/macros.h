/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once

#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname&) = delete; \
  classname& operator=(const classname&) = delete;

#define CACHELINE_SIZE 64

#define MYFRAME_MSG_TYPE_SUB "SUBSCRIBE"
#define MYFRAME_MSG_TYPE_PUB "PUBLISH"
#define MYFRAME_MSG_TYPE_TIMER "TIMER"

#define MYFRAME_ARG_KEY_SERVICE_LIB_DIR "app.lib_dir"
#define MYFRAME_ARG_KEY_THREAD_POOL_SIZE "app.thread_pool_size"
#define MYFRAME_ARG_KEY_WARNING_MSG_SIZE "app.warning_msg_size"
#define MYFRAME_ARG_KEY_PENDING_QUEUE_SIZE "app.pending_queue_size"
#define MYFRAME_ARG_KEY_RUN_QUEUE_SIZE "app.run_queue_size"
#define MYFRAME_ARG_KEY_EVENT_CONNE_SIZE "app.event_conn_size"
