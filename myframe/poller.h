/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <atomic>
#include <vector>

#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class MYFRAME_EXPORT Poller {
 public:
  Poller() = default;
  virtual ~Poller() = default;

  static std::shared_ptr<Poller> Create();

  virtual bool Init() = 0;
  virtual bool Add(const std::shared_ptr<Event>&) const = 0;
  virtual bool Del(const std::shared_ptr<Event>&) const = 0;
  virtual int Wait(std::vector<ev_handle_t>* evs, int timeout_ms = 100) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Poller)
};

}  // namespace myframe
