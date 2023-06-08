/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "myframe/list.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define MY_RESOLUTION_MS 10

namespace myframe {

class Timer final : public ListNode {
  friend class TimerManager;

 public:
  Timer() {}
  virtual ~Timer() {}

  std::string actor_name_;
  std::string timer_name_;
  uint32_t expire_;  // interval
  bool run_;
};

class TimerManager final {
 public:
  TimerManager();
  virtual ~TimerManager();

  int Timeout(
    const std::string& actor_name,
    const std::string& timer_name,
    int time);

  std::list<std::shared_ptr<Msg>>* Updatetime();

 private:
  void _AddTimerNode(Timer* node);
  void _Updatetime();
  void _Execute();
  void _MoveList(int level, int idx);
  void _Shift();
  void _Dispath(List* cur);

  List tv1_[TVR_SIZE];
  List tv2_[TVN_SIZE];
  List tv3_[TVN_SIZE];
  List tv4_[TVN_SIZE];
  List tv5_[TVN_SIZE];
  List* tv_[4];

  uint32_t time_;
  uint64_t cur_point_;

  std::list<std::shared_ptr<Msg>> timeout_list_;
  std::mutex mtx_;
};

class WorkerTimer final : public Worker {
  friend class App;

 public:
  WorkerTimer() = default;
  virtual ~WorkerTimer();

  int SetTimeout(
    const std::string& actor_name,
    const std::string& timer_name,
    int time);

  void Init() override;
  void Run() override;
  void Exit() override;
  EventType GetType() override {
    return EventType::kWorkerTimer;
  }

 private:
  int Work();
  int sleep_us_{2500};
  int cur_us_{0};
  int dispatch_timeout_us_{1000000};
  TimerManager timer_mgr_;
};

}  // namespace myframe
