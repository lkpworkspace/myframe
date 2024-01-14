/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <errno.h>
#include <sys/epoll.h>

#include <memory>
#include <atomic>
#include <vector>

#include "myframe/log.h"
#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/poller.h"

struct epoll_event;

namespace myframe {

class PollerLinux final : public Poller {
 public:
  PollerLinux() = default;
  virtual ~PollerLinux();

  bool Init() override;
  bool Add(const std::shared_ptr<Event>&) const override;
  bool Del(const std::shared_ptr<Event>&) const override;
  int Wait(std::vector<ev_handle_t>* evs, int timeout_ms = 100) override;

 private:
  std::atomic_bool init_{false};
  int poll_fd_{-1};
  size_t max_ev_count_{64};
  struct epoll_event* evs_{nullptr};

  DISALLOW_COPY_AND_ASSIGN(PollerLinux)
};

PollerLinux::~PollerLinux() {
  if (poll_fd_ != -1) {
    close(poll_fd_);
    poll_fd_ = -1;
  }
  if (evs_ != nullptr) {
    free(evs_);
    evs_ = nullptr;
  }
  init_.store(false);
}

bool PollerLinux::Init() {
  if (init_.load()) {
    return true;
  }
  poll_fd_ = epoll_create(1024);
  if (-1 == poll_fd_) {
    LOG(ERROR) << "poller create() failed, " << strerror(errno);
    return false;
  }
  LOG(INFO) << "Create epoll fd " << poll_fd_;
  auto void_evs = malloc(sizeof(struct epoll_event) * max_ev_count_);
  evs_ = reinterpret_cast<struct epoll_event*>(void_evs);
  init_.store(true);
  return true;
}

bool PollerLinux::Add(const std::shared_ptr<Event>& ev) const {
  if (!init_.load()) {
    return false;
  }
  struct epoll_event event;
  event.data.fd = ev->GetHandle();
  event.events = EPOLLIN;
  int res = 0;
  // 如果该事件已经注册，就修改事件类型
  res = epoll_ctl(poll_fd_, EPOLL_CTL_MOD, ev->GetHandle(), &event);
  if (-1 == res) {
    // 没有注册就添加至epoll
    res = epoll_ctl(poll_fd_, EPOLL_CTL_ADD, ev->GetHandle(), &event);
    if (-1 == res) {
      LOG(ERROR) << "epoll_ctl error, " << strerror(errno);
      return false;
    }
  } else {
    LOG(WARNING)
      << " has already reg ev " << ev->GetHandle() << ": "
      << strerror(errno);
    return false;
  }
  return true;
}

bool PollerLinux::Del(const std::shared_ptr<Event>& ev) const {
  if (!init_.load()) {
    return false;
  }
  if (-1 == epoll_ctl(poll_fd_, EPOLL_CTL_DEL, ev->GetHandle(), NULL)) {
    LOG(ERROR) << "del event " << ev->GetHandle() << ": " << strerror(errno);
    return false;
  }
  return true;
}

int PollerLinux::Wait(std::vector<ev_handle_t>* evs, int timeout_ms) {
  if (!init_.load()) {
    return -1;
  }
  evs->clear();
  int ev_count = epoll_wait(poll_fd_,
    evs_,
    static_cast<int>(max_ev_count_),
    timeout_ms);
  if (0 > ev_count) {
    LOG(WARNING) << "epoll wait error: " << strerror(errno);
    return -1;
  }
  for (int i = 0; i < ev_count; ++i) {
    if (evs_[i].events != EPOLLIN) {
      LOG(WARNING) << "epoll event " << evs_[i].events << " continue";
      continue;
    }
    evs->push_back(evs_[i].data.fd);
  }
  return ev_count;
}

}  // namespace myframe
