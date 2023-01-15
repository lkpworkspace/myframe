/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/app.h"

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>

#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/common.h"
#include "myframe/context.h"
#include "myframe/context_manager.h"
#include "myframe/event_conn.h"
#include "myframe/event_conn_manager.h"
#include "myframe/flags.h"
#include "myframe/mod_manager.h"
#include "myframe/msg.h"
#include "myframe/worker_common.h"
#include "myframe/worker_manager.h"
#include "myframe/worker_timer.h"
#include "myframe/mailbox.h"

namespace myframe {

std::shared_ptr<WorkerTimer> App::GetTimerWorker() {
  if (!worker_mgr_) {
    return nullptr;
  }
  auto w = worker_mgr_->Get(myframe::FLAGS_myframe_worker_timer_name);
  return std::dynamic_pointer_cast<WorkerTimer>(w);
}

App::App()
  : epoll_fd_(-1),
    context_mgr_(new ContextManager()),
    mods_(new ModManager()),
    worker_mgr_(new WorkerManager()),
    ev_conn_mgr_(new EventConnManager()) {}

App::~App() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
    epoll_fd_ = -1;
  }
  LOG(INFO) << "app deconstruct";
}

bool App::Init() {
  epoll_fd_ = epoll_create(1024);
  if (-1 == epoll_fd_) {
    LOG(ERROR) << strerror(errno);
    return false;
  }
  LOG(INFO) << "Create epoll fd " << epoll_fd_;

  bool ret = true;
  ret &=
      ev_conn_mgr_->Init(
        shared_from_this(),
        myframe::FLAGS_myframe_conn_count);
  ret &= StartCommonWorker(myframe::FLAGS_myframe_worker_count);
  ret &= StartTimerWorker();

  quit_ = false;
  return ret;
}

bool App::LoadModsFromConf(const std::string& path) {
  auto service_list = Common::GetDirFiles(path);
  LOG(INFO) << "Search " << service_list.size() << " service conf"
            << ", from " << path;
  if (service_list.empty()) {
    LOG(WARNING) << "Can't find service conf file,"
                 << " skip load from service conf file";
    return false;
  }
  bool res = false;
  for (const auto& it : service_list) {
    LOG(INFO) << "Load " << it << " ...";
    auto root = Common::LoadJsonFromFile(it);
    if (root.isNull()) {
      LOG(ERROR) << it << " parse failed, skip";
      continue;
    }
    if (!root.isMember("type") || !root["type"].isString()) {
      LOG(ERROR) << it << " key \"type\": no key or not string, skip";
      continue;
    }
    const auto& type = root["type"].asString();
    // load actor
    if (root.isMember("actor") && root["actor"].isObject()) {
      const auto& actor_list = root["actor"];
      Json::Value::Members actor_name_list = actor_list.getMemberNames();
      for (auto inst_name_it = actor_name_list.begin();
           inst_name_it != actor_name_list.end(); ++inst_name_it) {
        LOG(INFO) << "search actor " << *inst_name_it << " ...";
        if (type == "library") {
          res |= LoadActorFromLib(root, actor_list, *inst_name_it);
        } else if (type == "class") {
          res |= LoadActorFromClass(root, actor_list, *inst_name_it);
        } else {
          LOG(ERROR) << "Unknown type " << type;
        }
      }
    }
    // worker
    if (root.isMember("worker") && root["worker"].isObject()) {
      const auto& worker_list = root["worker"];
      Json::Value::Members worker_name_list = worker_list.getMemberNames();
      for (auto inst_name_it = worker_name_list.begin();
           inst_name_it != worker_name_list.end(); ++inst_name_it) {
        LOG(INFO) << "search worker " << *inst_name_it << " ...";
        if (type == "library") {
          res |= LoadWorkerFromLib(root, worker_list, *inst_name_it);
        } else if (type == "class") {
          res |= LoadWorkerFromClass(root, worker_list, *inst_name_it);
        } else {
          LOG(ERROR) << "Unknown type " << type;
        }
      }
    }
  }
  return res;
}

bool App::LoadActorFromLib(
  const Json::Value& root,
  const Json::Value& actor_list,
  const std::string& actor_name) {
  if (!root.isMember("lib") || !root["lib"].isString()) {
    LOG(ERROR) << "actor " << actor_name
               << " key \"lib\": no key or not string, skip";
    return false;
  }
  auto lib_dir = Common::GetAbsolutePath(FLAGS_myframe_lib_dir);
  const auto& lib_name = root["lib"].asString();
  if (!mods_->LoadMod(lib_dir + lib_name)) {
    LOG(ERROR) << "load lib " << lib_name << " failed, skip";
    return false;
  }

  const auto& insts = actor_list[actor_name];
  bool res = false;
  for (const auto& inst : insts) {
    LOG(INFO) << "create actor instance \"" << actor_name
              << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR) << "actor " << actor_name
                 << " key \"instance_name\": no key, skip";
      continue;
    }
    if (!inst.isMember("instance_params")) {
      LOG(ERROR) << "actor " << actor_name
                 << " key \"instance_params\": no key, skip";
      continue;
    }
    res |= CreateContext(lib_name, actor_name, inst["instance_name"].asString(),
                         inst["instance_params"].asString());
  }
  return res;
}

bool App::LoadActorFromClass(
  const Json::Value& root,
  const Json::Value& actor_list,
  const std::string& actor_name) {
  const auto& insts = actor_list[actor_name];
  bool res = false;
  for (const auto& inst : insts) {
    LOG(INFO) << "create instance \"class\""
              << ": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR) << "actor " << actor_name
                 << " key \"instance_name\": no key, skip";
      continue;
    }
    if (!inst.isMember("instance_params")) {
      LOG(ERROR) << "actor " << actor_name
                 << " key \"instance_params\": no key, skip";
      continue;
    }
    res |= CreateContext("class", actor_name, inst["instance_name"].asString(),
                         inst["instance_params"].asString());
  }
  return res;
}

bool App::LoadWorkerFromLib(
  const Json::Value& root,
  const Json::Value& worker_list,
  const std::string& worker_name) {
  if (!root.isMember("lib") || !root["lib"].isString()) {
    LOG(ERROR) << "worker \"" << worker_name
               << "\" key \"lib\": no key or not string, skip";
    return false;
  }
  auto lib_dir = Common::GetAbsolutePath(FLAGS_myframe_lib_dir);
  const auto& lib_name = root["lib"].asString();
  if (!mods_->LoadMod(lib_dir + lib_name)) {
    LOG(ERROR) << "load lib " << lib_name << " failed, skip";
    return false;
  }

  const auto& insts = worker_list[worker_name];
  bool res = false;
  for (const auto& inst : insts) {
    LOG(INFO) << "create worker instance \"" << worker_name
              << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR) << "worker " << worker_name
                 << " key \"instance_name\": no key, skip";
      continue;
    }
    res = true;
    auto worker = mods_->CreateWorkerInst(lib_name, worker_name);
    if (worker == nullptr) {
      LOG(ERROR) << "create worker " << lib_name << "." << worker_name << "."
                 << inst["instance_name"].asString() << " failed, continue";
      continue;
    }
    AddWorker(inst["instance_name"].asString(), worker);
  }
  return res;
}

bool App::LoadWorkerFromClass(
  const Json::Value& root,
  const Json::Value& worker_list,
  const std::string& worker_name) {
  const auto& insts = worker_list[worker_name];
  bool res = false;
  for (const auto& inst : insts) {
    LOG(INFO) << "create instance \"class\""
              << ": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR) << "worker \"" << worker_name
                 << "\" key \"instance_name\": no key, skip";
      continue;
    }
    res = true;
    auto worker = mods_->CreateWorkerInst("class", worker_name);
    if (worker == nullptr) {
      LOG(ERROR) << "create worker "
                 << "class." << worker_name << "."
                 << inst["instance_name"].asString() << " failed, continue";
      continue;
    }
    AddWorker(inst["instance_name"].asString(), worker);
  }
  return res;
}

bool App::AddActor(
  const std::string& inst_name,
  const std::string& params,
  std::shared_ptr<Actor> actor) {
  actor->SetInstName(inst_name);
  return CreateContext(actor, params);
}

bool App::AddWorker(
  const std::string& inst_name,
  std::shared_ptr<Worker> worker) {
  worker->SetInstName(inst_name);
  if (!worker_mgr_->Add(worker)) {
    return false;
  }
  if (!AddEvent(std::dynamic_pointer_cast<Event>(worker))) {
    return false;
  }
  worker->Start();
  return true;
}

const std::shared_ptr<const Msg> App::SendRequest(
  const std::string& name,
  std::shared_ptr<Msg> msg) {
  auto conn = ev_conn_mgr_->Get();
  auto resp = conn->SendRequest(name, msg);
  ev_conn_mgr_->Release(conn);
  return resp;
}

/**
 * 创建一个新的actor:
 *      1. 从ModManager中获得对应模块对象
 *      2. 生成Context
 *      3. 将模块对象加入Context对象
 *      4. 将Context加入Context数组
 *      3. 注册句柄
 *      4. 初始化actor
 */
bool App::CreateContext(
  const std::string& mod_name,
  const std::string& actor_name,
  const std::string& instance_name,
  const std::string& params) {
  auto mod_inst = mods_->CreateActorInst(mod_name, actor_name);
  if (mod_inst == nullptr) {
    LOG(ERROR) << "Create mod " << mod_name << "." << actor_name << " failed";
    return false;
  }
  mod_inst->SetInstName(instance_name);
  return CreateContext(mod_inst, params);
}

bool App::CreateContext(
  std::shared_ptr<Actor> mod_inst,
  const std::string& params) {
  auto ctx = std::make_shared<Context>(shared_from_this(), mod_inst);
  if (ctx->Init(params.c_str())) {
    LOG(ERROR) << "init " << mod_inst->GetActorName() << " fail";
    return false;
  }
  context_mgr_->RegContext(ctx);
  // 初始化之后, 手动将actor中发送消息队列分发出去
  DispatchMsg(ctx);
  return true;
}

bool App::AddEvent(std::shared_ptr<Event> ev) {
  struct epoll_event event;
  event.data.fd = ev->GetFd();
  event.events = ev->ListenEpollEventType();
  int res = 0;
  // 如果该事件已经注册，就修改事件类型
  if (-1 == (res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ev->GetFd(), &event))) {
    // 没有注册就添加至epoll
    if (-1 ==
        (res = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, ev->GetFd(), &event))) {
      LOG(ERROR) << "epoll " << strerror(errno);
      return false;
    }
  } else {
    LOG(WARNING) << " has already reg ev " << ev->GetFd() << ": "
                 << strerror(errno);
    return false;
  }
  return true;
}

bool App::DelEvent(std::shared_ptr<Event> ev) {
  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, ev->GetFd(), NULL)) {
    LOG(ERROR) << "del event " << ev->GetFd() << ": " << strerror(errno);
    return false;
  }
  return true;
}

bool App::StartCommonWorker(int worker_count) {
  bool ret = false;
  for (int i = 0; i < worker_count; ++i) {
    auto worker = std::make_shared<WorkerCommon>();
    worker->SetModName("class");
    worker->SetTypeName("WorkerCommon");
    if (!AddWorker(std::to_string(i), worker)) {
      LOG(ERROR) << "start common worker " << i << " failed";
      continue;
    }
    LOG(INFO) << "start common worker " << worker->GetWorkerName();
    ret = true;
  }
  return ret;
}

bool App::StartTimerWorker() {
  auto worker = std::make_shared<WorkerTimer>();
  worker->SetModName("class");
  worker->SetTypeName("timer");
  if (!AddWorker("#1", worker)) {
    LOG(ERROR) << "start timer worker failed";
    return false;
  }
  LOG(INFO) << "start timer worker " << worker->GetWorkerName();
  return true;
}

void App::DispatchMsg(std::list<std::shared_ptr<Msg>>* msg_list) {
  LOG_IF(WARNING,
      msg_list->size() > myframe::FLAGS_myframe_dispatch_or_process_msg_max)
    << " dispatch msg too many";
  std::lock_guard<std::mutex> lock(dispatch_mtx_);
  for (auto& msg : (*msg_list)) {
    DLOG(INFO) << *msg;
    auto name_list = SplitMsgName(msg->GetDst());
    if (name_list.size() < 2) {
      LOG(ERROR) << "Unknown msg " << *msg;
      continue;
    }

    if (name_list[0] == "worker") {
      // dispatch to user worker
      worker_mgr_->DispatchWorkerMsg(msg);
    } else if (name_list[0] == "actor") {
      // dispatch to actor
      context_mgr_->DispatchMsg(msg);
    } else if (name_list[0] == "event") {
      if (name_list[1] == "conn") {
        ev_conn_mgr_->Notify(msg->GetDst(), msg);
      } else {
        LOG(ERROR) << "Unknown msg " << *msg;
      }
    } else {
      LOG(ERROR) << "Unknown msg " << *msg;
    }
  }
  msg_list->clear();
}

// 将获得的消息分发给其他actor
void App::DispatchMsg(std::shared_ptr<Context> context) {
  if (nullptr == context) {
    return;
  }
  DLOG(INFO) << context->GetActor()->GetActorName() << " dispatch msg...";
  context->SetRuningFlag(false);
  auto msg_list = context->GetMailbox()->GetSendList();
  DispatchMsg(msg_list);
}

void App::CheckStopWorkers() {
  DLOG(INFO) << "check stop worker";
  worker_mgr_->WeakupWorker();

  LOG_IF(INFO, worker_mgr_->IdleWorkerSize() == 0)
      << "worker busy, wait for idle worker...";
  std::shared_ptr<Context> context = nullptr;
  std::shared_ptr<Worker> worker = nullptr;
  while ((worker = worker_mgr_->FrontIdleWorker()) != nullptr) {
    if (nullptr == (context = context_mgr_->GetContextWithMsg())) {
      DLOG(INFO) << "no actor need process, waiting...";
      break;
    }
    DLOG(INFO) << worker->GetWorkerName() << "."
               << worker->GetPosixThreadId()
               << " dispatch task to idle worker";
    auto msg_list = context->GetMailbox()->GetRecvList();
    if (!msg_list->empty()) {
      LOG_IF(WARNING, msg_list->size() >
                          myframe::FLAGS_myframe_dispatch_or_process_msg_max)
          << context->GetActor()->GetActorName()
          << " recv msg size too many: " << msg_list->size();
      DLOG(INFO) << "run " << context->GetActor()->GetActorName();
      worker->GetMailbox()->Recv(msg_list);
      DLOG(INFO) << context->GetActor()->GetActorName()
        << " has " << worker->GetMailbox()->RecvSize() << " msg need process";
      worker_mgr_->PopFrontIdleWorker();
      auto common_idle_worker =
          std::dynamic_pointer_cast<WorkerCommon>(worker);
      common_idle_worker->SetContext(context);
      worker->SendCmdToWorker(WorkerCmd::RUN);
    } else {
      LOG(ERROR) << context->GetActor()->GetActorName() << " has no msg";
    }
  }
}

void App::ProcessTimerEvent(std::shared_ptr<WorkerTimer> timer_worker) {
  // 将定时器线程的发送队列分发完毕
  DLOG(INFO) << timer_worker->GetWorkerName() << " dispatch msg...";
  DispatchMsg(timer_worker->GetMailbox()->GetSendList());

  WorkerCmd cmd;
  timer_worker->RecvCmdFromWorker(&cmd);
  switch (cmd) {
    case WorkerCmd::IDLE:  // idle
      DLOG(INFO) << timer_worker->GetWorkerName() << " run again";
      timer_worker->SendCmdToWorker(WorkerCmd::RUN);
      break;
    case WorkerCmd::QUIT:  // quit
      LOG(INFO) << timer_worker->GetWorkerName()
                << " quit, delete from myframe";
      DelEvent(timer_worker);
      worker_mgr_->Del(timer_worker);
      break;
    default:
      LOG(WARNING) << "Unknown timer worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

void App::ProcessUserEvent(std::shared_ptr<Worker> worker) {
  // 将用户线程的发送队列分发完毕
  DLOG(INFO) << worker->GetWorkerName() << " dispatch msg...";
  DispatchMsg(worker->GetMailbox()->GetSendList());

  WorkerCmd cmd;
  worker->RecvCmdFromWorker(&cmd);
  switch (cmd) {
    case WorkerCmd::IDLE:  // idle
      DLOG(INFO) << worker->GetWorkerName() << " run again";
      worker->SendCmdToWorker(WorkerCmd::RUN);
      break;
    case WorkerCmd::WAIT_FOR_MSG:
      DLOG(INFO) << worker->GetWorkerName() << " wait for msg...";
      worker_mgr_->PushWaitWorker(worker);
      break;
    case WorkerCmd::QUIT:  // quit
      LOG(INFO) << worker->GetWorkerName() << " quit, delete from myframe";
      DelEvent(worker);
      worker_mgr_->Del(worker);
      break;
    default:
      LOG(WARNING) << "Unknown user worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

/// FIXME: Idle/DispatchMsg 会影响actor的执行顺序
void App::ProcessWorkerEvent(std::shared_ptr<WorkerCommon> worker) {
  // 将actor的发送队列分发完毕
  DLOG_IF(INFO, worker->GetContext() != nullptr)
      << worker->GetWorkerName() << "."
      << worker->GetPosixThreadId() << " dispatch "
      << worker->GetContext()->GetActor()->GetActorName() << " msg...";
  LOG_IF(WARNING, worker->GetContext() == nullptr)
      << worker->GetWorkerName() << "."
      << worker->GetPosixThreadId() << " no context";
  DispatchMsg(worker->GetContext());

  WorkerCmd cmd;
  worker->RecvCmdFromWorker(&cmd);
  switch (cmd) {
    case WorkerCmd::IDLE:  // idle
      // 将工作线程中的actor状态设置为全局状态
      // 将线程加入空闲队列
      DLOG(INFO) << worker->GetWorkerName() << "."
                 << worker->GetPosixThreadId()
                 << " idle, push to idle queue";
      worker->Idle();
      worker_mgr_->PushBackIdleWorker(worker);
      break;
    case WorkerCmd::QUIT:  // quit
      LOG(INFO) << worker->GetWorkerName() << "."
                << worker->GetPosixThreadId()
                << " quit, delete from myframe";
      DelEvent(std::dynamic_pointer_cast<Event>(worker));
      worker_mgr_->Del(worker);
      // FIXME: 应该将worker加入删除队列，等worker运行结束后再从队列删除
      // 否则会造成删除智能指针后，worker还没结束运行造成coredump
      break;
    default:
      LOG(WARNING) << "unknown common worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

void App::ProcessEventConn(std::shared_ptr<EventConn> ev) {
  // 将event_conn的发送队列分发完毕
  DispatchMsg(ev->GetMailbox()->GetSendList());

  WorkerCmd cmd;
  ev->RecvCmdFromWorker(&cmd);
  switch (cmd) {
    case WorkerCmd::IDLE:
      // do nothing
      break;
    case WorkerCmd::RUN:
      // do nothing
      break;
    default:
      LOG(WARNING) << "unknown common worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

void App::ProcessEvent(struct epoll_event* evs, int ev_count) {
  DLOG_IF(INFO, ev_count > 0) << "get " << ev_count << " event";
  for (int i = 0; i < ev_count; ++i) {
    std::shared_ptr<Event> ev_obj = nullptr;
    ev_obj = worker_mgr_->Get(evs[i].data.fd);
    if (ev_obj == nullptr) {
      ev_obj = ev_conn_mgr_->Get(evs[i].data.fd);
      if (ev_obj == nullptr) {
        LOG(ERROR) << "can't find ev obj, handle " << evs[i].data.fd;
        continue;
      }
    }
    ev_obj->RetEpollEventType(evs[i].events);
    switch (ev_obj->GetType()) {
      case EventType::WORKER_COMMON:
        ProcessWorkerEvent(std::dynamic_pointer_cast<WorkerCommon>(ev_obj));
        break;
      case EventType::WORKER_TIMER:
        ProcessTimerEvent(std::dynamic_pointer_cast<WorkerTimer>(ev_obj));
        break;
      case EventType::WORKER_USER:
        ProcessUserEvent(std::dynamic_pointer_cast<Worker>(ev_obj));
        break;
      case EventType::EVENT_CONN:
        ProcessEventConn(std::dynamic_pointer_cast<EventConn>(ev_obj));
        break;
      default:
        LOG(WARNING) << "unknown event";
        break;
    }
  }
}

int App::Exec() {
  int ev_count = 0;
  int max_ev_count = 64;
  int time_wait_ms = 1000;
  struct epoll_event* evs;
  evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * max_ev_count);

  while (worker_mgr_->WorkerSize()) {
    /// 检查空闲线程队列是否有空闲线程，如果有就找到一个有消息的actor处理
    CheckStopWorkers();
    /// 等待事件
    ev_count = epoll_wait(epoll_fd_, evs, max_ev_count, time_wait_ms);
    if (0 > ev_count) {
      LOG(ERROR) << "epoll wait error: " << strerror(errno);
    }
    /// 处理事件
    ProcessEvent(evs, ev_count);
  }

  // quit App
  free(evs);
  close(epoll_fd_);
  epoll_fd_ = -1;
  LOG(INFO) << "app exit exec";
  return 0;
}

}  // namespace myframe
