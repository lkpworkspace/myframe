/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#include <list>
#include <condition_variable>

#include <glog/logging.h>

#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/cmd_channel.h"

namespace myframe {

class CmdChannelGeneric final : public CmdChannel {
 public:
  explicit CmdChannelGeneric(std::shared_ptr<Poller>);
  virtual ~CmdChannelGeneric();

  ev_handle_t GetOwnerHandle() const override;
  ev_handle_t GetMainHandle() const override;

  int SendToOwner(const Cmd& cmd) override;
  int RecvFromOwner(Cmd* cmd) override;

  int SendToMain(const Cmd& cmd) override;
  int RecvFromMain(Cmd* cmd, int timeout_ms = -1) override;

 private:
  std::mutex main_cmd_mtx_;
  std::list<Cmd> to_main_cmd_;

  std::mutex mtx_;
  std::list<Cmd> to_owner_cmd_;
  std::condition_variable cv_;

  DISALLOW_COPY_AND_ASSIGN(CmdChannelGeneric)
};

CmdChannelGeneric::CmdChannelGeneric(std::shared_ptr<Poller> poller)
  : CmdChannel(poller) {
}

CmdChannelGeneric::~CmdChannelGeneric() {
  LOG(INFO) << "CmdChannel " << this << " deconstruct";
}

ev_handle_t CmdChannelGeneric::GetOwnerHandle() const {
  return reinterpret_cast<ev_handle_t>(const_cast<CmdChannelGeneric*>(this));
}

ev_handle_t CmdChannelGeneric::GetMainHandle() const {
  return reinterpret_cast<ev_handle_t>(const_cast<CmdChannelGeneric*>(this));
}

int CmdChannelGeneric::SendToOwner(const Cmd& cmd) {
  std::lock_guard<std::mutex> lk(mtx_);
  to_owner_cmd_.push_back(cmd);
  cv_.notify_one();
  return 0;
}

int CmdChannelGeneric::RecvFromOwner(Cmd* cmd) {
  std::lock_guard<std::mutex> lk(main_cmd_mtx_);
  *cmd = to_main_cmd_.front();
  to_main_cmd_.pop_front();
  return 0;
}

int CmdChannelGeneric::RecvFromMain(Cmd* cmd, int timeout_ms) {
  std::unique_lock<std::mutex> lk(mtx_);
  using namespace std::chrono_literals;  // NOLINT
  if (timeout_ms > 0) {
    cv_.wait_for(lk, timeout_ms * 1ms,
      [this](){ return !to_owner_cmd_.empty(); });
  } else {
    cv_.wait(lk, [this](){ return !to_owner_cmd_.empty(); });
  }
  if (to_owner_cmd_.size() > 1) {
    std::stringstream ss;
    for (Cmd p : to_owner_cmd_) {
      ss << static_cast<char>(p) << ", ";
    }
    LOG(WARNING) << this << " too many cmd " << ss.str();
  }
  *cmd = to_owner_cmd_.front();
  to_owner_cmd_.pop_front();
  return 0;
}

int CmdChannelGeneric::SendToMain(const Cmd& cmd) {
  std::lock_guard<std::mutex> lk(main_cmd_mtx_);
  to_main_cmd_.push_back(cmd);

  poller_->Notify(
    reinterpret_cast<ev_handle_t>(
      const_cast<CmdChannelGeneric*>(this)));
  return 0;
}

}  // namespace myframe
