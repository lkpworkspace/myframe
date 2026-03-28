/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <memory>
#include "myframe/msg.h"

namespace pymyframe {

class Msg {
  friend class App;
 public:
  enum class TransMode : int {
    HYBRID = 0,
    INTRA = 1,
    DDS = 2,
  };

  Msg() {
    msg_ = std::make_shared<myframe::Msg>();
    // std::cout << "msg construct\n";
  }

  Msg(
    const std::string& dst,
    const std::string& data) {
    msg_ = std::make_shared<myframe::Msg>();
    setDst(dst);
    setData(data);
    // std::cout << "msg (" << data << ") construct\n";
  }

  ~Msg() {
    // std::cout << "msg (" << getData() << ") deconstruct\n";
  }

  TransMode getTransMode() const {
    auto msg_tm = msg_->GetTransMode();
    if (msg_tm == myframe::Msg::TransMode::kHybrid) {
      return TransMode::HYBRID;
    } else if (msg_tm == myframe::Msg::TransMode::kDDS) {
      return TransMode::DDS;
    } else {
      return TransMode::INTRA;
    }
  }
  const std::string& getSrc() const { return msg_->GetSrc(); }
  const std::string& getDst() const { return msg_->GetDst(); }
  const std::string& getType() const { return msg_->GetType(); }
  const std::string& getDesc() const { return msg_->GetDesc(); }
  const std::string& getData() const { return msg_->GetData(); }

  void setTransMode(TransMode tans_mode) {
    myframe::Msg::TransMode msg_tm;
    if (tans_mode == TransMode::HYBRID) {
      msg_tm = myframe::Msg::TransMode::kHybrid;
    } else if (tans_mode == TransMode::DDS) {
      msg_tm = myframe::Msg::TransMode::kDDS;
    } else {
      msg_tm = myframe::Msg::TransMode::kIntra;
    }
    msg_->SetTransMode(msg_tm);
  }
  void setSrc(const std::string& src) { msg_->SetSrc(src); }
  void setDst(const std::string& dst) { msg_->SetDst(dst); }
  void setType(const std::string& type) { msg_->SetType(type); }
  void setDesc(const std::string& desc) { msg_->SetDesc(desc); }
  void setData(const std::string& data) { msg_->SetData(data); }

 private:
  std::shared_ptr<myframe::Msg> msg_;
};

}  // namespace pymyframe
