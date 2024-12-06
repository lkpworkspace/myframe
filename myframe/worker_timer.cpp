/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_timer.h"

#include <chrono>
#include <thread>
#include <utility>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/app.h"

namespace myframe {

uint64_t TimerManager::GetMonoTimeMs() {
  auto now = std::chrono::steady_clock::now();
  return static_cast<uint64_t>(now.time_since_epoch().count() / 1e6);
}

TimerManager::TimerManager() {
  tv_[0] = tv2_;
  tv_[1] = tv3_;
  tv_[2] = tv4_;
  tv_[3] = tv5_;

  cur_point_ = GetMonoTimeMs() / 10;
}

TimerManager::~TimerManager() {}

void TimerManager::_AddTimerNode(Timer* node) {
  uint32_t time = node->expire_;
  uint32_t cur_time = time_;

  if ((time | TVR_MASK) == (cur_time | TVR_MASK)) {
    tv1_[time & TVR_MASK].AddTail(node);
  } else {
    int i;
    uint32_t mask = TVR_SIZE << TVN_BITS;
    for (i = 0; i < 3; ++i) {
      if ((time | (mask - 1)) == (cur_time | (mask - 1))) {
        break;
      }
      mask <<= TVN_BITS;
    }
    tv_[i][((time >> (TVR_BITS + i * TVN_BITS)) & TVN_MASK)].AddTail(node);
  }
}

int TimerManager::Timeout(
    const std::string& actor_name,
    const std::string& timer_name, int time) {
  if (time <= 0) return -1;
  Timer* timer = new Timer();
  timer->actor_name_ = actor_name;
  timer->timer_name_ = timer_name;

  // add node
  mtx_.lock();
  timer->expire_ = time + time_;
  _AddTimerNode(timer);
  mtx_.unlock();
  return 0;
}

void TimerManager::_Dispath(List* cur) {
  Timer* timer;
  ListNode* begin;
  ListNode* end;
  ListNode* temp;

  begin = cur->Begin();
  end = cur->End();
  for (; begin != end;) {
    temp = begin->next;
    cur->Del(begin);
    timer = dynamic_cast<Timer*>(begin);
    auto msg = std::make_shared<Msg>();
    msg->SetSrc("worker.timer");
    msg->SetDst(timer->actor_name_);
    msg->SetDesc(timer->timer_name_);
    msg->SetType("TIMER");
    delete begin;
    timeout_list_.push_back(std::move(msg));
    begin = temp;
  }
}

void TimerManager::_Execute() {
  int idx = time_ & TVR_MASK;
  while (!tv1_[idx].IsEmpty()) {
    _Dispath(&tv1_[idx]);
  }
}
void TimerManager::_MoveList(int level, int idx) {
  List* cur = &tv_[level][idx];
  Timer* timer;
  ListNode* begin;
  ListNode* end;
  ListNode* temp;

  begin = cur->Begin();
  end = cur->End();
  for (; begin != end;) {
    temp = begin->next;
    cur->Del(begin);
    timer = dynamic_cast<Timer*>(begin);
    _AddTimerNode(timer);
    begin = temp;
  }
}
void TimerManager::_Shift() {
  int mask = TVR_SIZE;
  uint32_t ct = ++time_;
  if (ct == 0) {
    _MoveList(3, 0);
  } else {
    uint32_t time = ct >> TVR_BITS;
    int i = 0;
    // 每255个滴答需要重新分配定时器所在区间
    while ((ct & (mask - 1)) == 0) {
      int idx = time & TVN_MASK;
      if (idx != 0) {
        _MoveList(i, idx);
        break;
      }
      mask <<= TVN_BITS;
      time >>= TVN_BITS;
      ++i;
    }
  }
}
void TimerManager::_Updatetime() {
  mtx_.lock();
  _Execute();
  _Shift();
  _Execute();
  mtx_.unlock();
}
std::list<std::shared_ptr<Msg>>* TimerManager::Updatetime() {
  uint64_t cp = GetMonoTimeMs() / MY_RESOLUTION_MS;
  if (cp < cur_point_) {
    LOG(ERROR) << "Future time: " << cp << ":" << cur_point_;
    cur_point_ = cp;
  } else if (cp != cur_point_) {
    uint32_t diff = static_cast<uint32_t>(cp - cur_point_);
    cur_point_ = cp;
    for (std::size_t i = 0; i < diff; ++i) {
      _Updatetime();
    }
  }
  return &timeout_list_;
}

//////////////////////////////////////////////////////

WorkerTimer::~WorkerTimer() {}

void WorkerTimer::Run() {
  int dispatch = Work();
  if (dispatch || cur_us_ >= dispatch_timeout_us_) {
    DispatchMsg();
    cur_us_ = 0;
  }
  std::this_thread::sleep_for(std::chrono::microseconds(sleep_us_));
  cur_us_ += sleep_us_;
}

void WorkerTimer::Init() {
  LOG(INFO) << "timer worker " << GetWorkerName() << " init";
}

void WorkerTimer::Exit() {
  LOG(INFO) << "timer worker " << GetWorkerName() << " exit";
}

int WorkerTimer::SetTimeout(
  const std::string& actor_name,
  const std::string& timer_name,
  int time) {
  VLOG(1) << actor_name << " set timeout(" << timer_name
             << "): " << (time * 10) << "ms";
  return timer_mgr_.Timeout(actor_name, timer_name, time);
}

int WorkerTimer::Work() {
  auto timeout_list = timer_mgr_.Updatetime();
  GetMailbox()->Send(timeout_list);
  return GetMailbox()->SendSize();
}

}  // namespace myframe
