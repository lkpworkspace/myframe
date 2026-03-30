/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <Python.h>
#include <string>
#include <memory>
#include <utility>
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "pymsg.h"
#include "pyactor.h"

namespace pymyframe {

class PyActor : public myframe::Actor {
 public:
  PyActor() {
    // std::cout << "pyactor construct\n";
  }

  virtual ~PyActor() {
    // std::cout << GetActorName() << " deconstruct\n";
  }

  int Init() {
    int res;
    PyGILState_STATE gstate = PyGILState_Ensure();
    res = actor_->init();
    PyGILState_Release(gstate);
    return res;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) {
    pymyframe::Msg m;
    MsgConvert1(msg, &m);

    PyGILState_STATE gstate = PyGILState_Ensure();
    actor_->proc(m);
    PyGILState_Release(gstate);
  }

  void SetPyActor(pymyframe::Actor* obj) {
    actor_ = obj;
    actor_->setTimeoutCallback([this](const std::string& timer_name, int ms){
      return Timeout(timer_name, ms / 10);
    });
    actor_->setSendCallback([this](const pymyframe::Msg& msg){
      auto mailbox = GetMailbox();
      auto cmsg = MsgConvert2(msg);
      if (cmsg->GetSrc().empty()) {
        cmsg->SetSrc(GetActorName());
      }
      return mailbox->Send(std::move(cmsg));
    });
    actor_->setSubscribeCallback([this](
        const std::string& addr,
        const std::string& desc,
        const Msg::TransMode tm) {
      return Subscribe(addr, desc, TransModeConvert2(tm));
    });
    actor_->setGetActorNameCallback([this]() {
      return GetActorName();
    });
  }

 private:
  pymyframe::Msg::TransMode TransModeConvert1(
      const myframe::Msg::TransMode tm_in) {
    pymyframe::Msg::TransMode tm_out;
    if (tm_in == myframe::Msg::TransMode::kHybrid) {
      tm_out = pymyframe::Msg::TransMode::HYBRID;
    } else if (tm_in == myframe::Msg::TransMode::kDDS) {
      tm_out = pymyframe::Msg::TransMode::DDS;
    } else {
      tm_out = pymyframe::Msg::TransMode::INTRA;
    }
    return tm_out;
  }

  void MsgConvert1(
      const std::shared_ptr<const myframe::Msg>& msg_in,
      pymyframe::Msg* msg_out) {
    myframe::Msg::TransMode msg_tm = msg_in->GetTransMode();
    msg_out->setTransMode(TransModeConvert1(msg_tm));
    msg_out->setSrc(msg_in->GetSrc());
    msg_out->setDst(msg_in->GetDst());
    msg_out->setName(msg_in->GetName());
    msg_out->setType(msg_in->GetType());
    msg_out->setDesc(msg_in->GetDesc());
    msg_out->setData(msg_in->GetData());
  }

  myframe::Msg::TransMode TransModeConvert2(
      const pymyframe::Msg::TransMode tm_in) {
    myframe::Msg::TransMode tm_out;
    if (tm_in == pymyframe::Msg::TransMode::HYBRID) {
      tm_out = myframe::Msg::TransMode::kHybrid;
    } else if (tm_in == pymyframe::Msg::TransMode::DDS) {
      tm_out = myframe::Msg::TransMode::kDDS;
    } else {
      tm_out = myframe::Msg::TransMode::kIntra;
    }
    return tm_out;
  }

  std::shared_ptr<myframe::Msg> MsgConvert2(const pymyframe::Msg& msg_in) {
    auto msg_out = std::make_shared<myframe::Msg>();
    pymyframe::Msg::TransMode pymsg_tm = msg_in.getTransMode();
    msg_out->SetTransMode(TransModeConvert2(pymsg_tm));
    msg_out->SetSrc(msg_in.getSrc());
    msg_out->SetDst(msg_in.getDst());
    msg_out->SetName(msg_in.getName());
    msg_out->SetType(msg_in.getType());
    msg_out->SetDesc(msg_in.getDesc());
    msg_out->SetData(msg_in.getData());
    return msg_out;
  }

  pymyframe::Actor* actor_{nullptr};
};

}  // namespace pymyframe
