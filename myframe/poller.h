/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <atomic>
#include <vector>

#include "myframe/macros.h"
#include "myframe/event.h"

struct epoll_event;

namespace myframe {

class Poller final {
 public:
  explicit Poller() = default;
  ~Poller();

  bool Init();
  bool Add(const std::shared_ptr<Event>&) const;
  bool Del(const std::shared_ptr<Event>&) const;
  int Wait(std::vector<ev_handle_t>* evs, int timeout_ms = 100);

 private:
  std::atomic_bool init_{false};
  int poll_fd_{-1};
  size_t max_ev_count_{64};
  struct epoll_event* evs_{nullptr};

  DISALLOW_COPY_AND_ASSIGN(Poller)
};

}  // namespace myframe
