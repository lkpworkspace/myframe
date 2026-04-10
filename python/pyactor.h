/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <memory>
#include <functional>
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "pymsg.h"

namespace pymyframe {

class PyActor;
class Actor {
  friend class PyActor;

 public:
  Actor() {
    // std::cout << "pymyframe actor construct\n";
  }

  virtual ~Actor() {
    // std::cout << "pymyframe actor deconstruct\n";
  }

  virtual int init() = 0;

  virtual void proc(const pymyframe::Msg& msg) = 0;

  int timeout(const std::string& timer_name, int ms) {
    if (timeout_cb_) {
      return timeout_cb_(timer_name, ms);
    }
    return -1;
  }

  void send(const pymyframe::Msg& msg) {
    if (send_cb_) {
      send_cb_(msg);
    }
  }

  bool subscribe(
      const std::string& addr,
      const std::string& msg_name,
      const Msg::TransMode mode = Msg::TransMode::INTRA) {
    if (subscribe_cb_) {
      return subscribe_cb_(addr, msg_name, mode);
    }
    return false;
  }

  bool publish(
      const std::string& data,
      const std::string& msg_name = "",
      const Msg::TransMode mode = Msg::TransMode::INTRA) {
    if (publish_cb_) {
      return publish_cb_(data, msg_name, mode);
    }
    return false;
  }

  const std::string getActorName() const {
    if (get_actor_name_cb_) {
      return get_actor_name_cb_();
    }
    return "";
  }

 private:
  void setTimeoutCallback(std::function<int(const std::string&, int)> cb) {
    timeout_cb_ = cb;
  }

  void setSendCallback(std::function<void(const pymyframe::Msg& msg)> cb) {
    send_cb_ = cb;
  }

  void setSubscribeCallback(
    std::function<
      bool(const std::string&, const std::string&, const Msg::TransMode)> cb) {
    subscribe_cb_ = cb;
  }

  void setPublishCallback(
    std::function<
      bool(const std::string&, const std::string&, const Msg::TransMode)> cb) {
    publish_cb_ = cb;
  }

  void setGetActorNameCallback(std::function<const std::string(void)> cb) {
    get_actor_name_cb_ = cb;
  }

  std::function<int(const std::string&, int)> timeout_cb_;
  std::function<void(const pymyframe::Msg& msg)> send_cb_;
  std::function<
    bool(const std::string&, const std::string&, const Msg::TransMode)>
      subscribe_cb_;
  std::function<
    bool(const std::string&, const std::string&, const Msg::TransMode)>
      publish_cb_;
  std::function<const std::string(void)> get_actor_name_cb_;
};

}  // namespace pymyframe
