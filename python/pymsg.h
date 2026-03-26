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
  Msg(
    const std::string& dst,
    const std::string& data) {
    msg_ = std::make_shared<myframe::Msg>();
    msg_->SetDst(dst);
    msg_->SetData(data);
    // std::cout << "msg construct\n";
  }

  ~Msg() {
    // std::cout << "msg deconstruct\n";
  }

 private:
  std::shared_ptr<myframe::Msg> msg_;
};

}  // namespace pymyframe
