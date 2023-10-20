/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <glog/logging.h>

#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/poller.h"

namespace myframe {

class PollerGeneric final : public Poller {
 public:
  PollerGeneric() = default;
  virtual ~PollerGeneric();

  bool Init() override;
  int Wait(std::vector<ev_handle_t>* evs, int timeout_ms = 100) override;
  void Notify(ev_handle_t h) override;

 private:
  std::vector<ev_handle_t> evs_;
  std::mutex mtx_;
  std::condition_variable cv_;

  DISALLOW_COPY_AND_ASSIGN(PollerGeneric)
};

PollerGeneric::~PollerGeneric() {
  LOG(INFO) << "poller deconstruct";
}

bool PollerGeneric::Init() {
  return true;
}

int PollerGeneric::Wait(std::vector<ev_handle_t>* evs, int timeout_ms) {
  evs->clear();
  using namespace std::chrono_literals;  // NOLINT
  std::unique_lock<std::mutex> lk(mtx_);
  if (timeout_ms > 0) {
    cv_.wait_for(lk, timeout_ms * 1ms, [this](){ return !evs_.empty(); });
  } else {
    cv_.wait(lk, [this](){ return !evs_.empty(); });
  }
  for (auto it = evs_.begin(); it != evs_.end(); ++it) {
      evs->push_back(*it);
  }
  evs_.clear();
  return evs->size();
}

void PollerGeneric::Notify(ev_handle_t h) {
  std::lock_guard<std::mutex> lk(mtx_);
  evs_.push_back(h);
  cv_.notify_one();
}

}  // namespace myframe
