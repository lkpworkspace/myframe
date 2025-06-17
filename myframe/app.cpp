/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/app.h"

#include <utility>

#include "myframe/log.h"
#include "myframe/platform.h"
#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/mailbox.h"
#include "myframe/actor.h"
#include "myframe/actor_context.h"
#include "myframe/actor_context_manager.h"
#include "myframe/event_manager.h"
#include "myframe/event_conn.h"
#include "myframe/event_conn_manager.h"
#include "myframe/worker_context.h"
#include "myframe/worker_common.h"
#include "myframe/worker_timer.h"
#include "myframe/worker_context_manager.h"
#include "myframe/mod_manager.h"
#include "myframe/poller.h"

namespace myframe {

std::shared_ptr<WorkerTimer> App::GetTimerWorker() {
  if (!worker_ctx_mgr_) {
    LOG(ERROR) << "worker context manager is nullptr";
    return nullptr;
  }
  std::string worker_timer_name = "worker.T.1";
  auto w = ev_mgr_->Get<WorkerContext>(worker_timer_name);
  if (w == nullptr) {
    LOG(ERROR)
      << "can't find " << worker_timer_name;
    return nullptr;
  }
  auto timer_worker = w->GetWorker<WorkerTimer>();
  return timer_worker;
}

App::App()
  : mods_(new ModManager())
  , poller_(Poller::Create())
  , actor_ctx_mgr_(new ActorContextManager())
  , ev_mgr_(new EventManager())
  , ev_conn_mgr_(new EventConnManager(ev_mgr_, poller_))
  , worker_ctx_mgr_(new WorkerContextManager(ev_mgr_)) {
  name_list_.reserve(3);
  LOG(INFO) << "myframe version: " << MYFRAME_VERSION;
}

App::~App() {
  LOG(INFO) << "app deconstruct";
}

bool App::Init(
  const std::string& lib_dir,
  int thread_pool_size,
  int event_conn_size,
  int warning_msg_size,
  int default_pending_queue_size,
  int default_run_queue_size) {
  if (state_.load() != State::kUninitialized) {
    return true;
  }

  bool ret = true;
  lib_dir_ = lib_dir;
  warning_msg_size_.store(warning_msg_size);
  default_pending_queue_size_ = default_pending_queue_size;
  default_run_queue_size_ = default_run_queue_size;
  ret &= poller_->Init();
  ret &= worker_ctx_mgr_->Init(warning_msg_size);
  ret &= ev_conn_mgr_->Init(event_conn_size);

  state_.store(State::kInitialized);

  ret &= StartCommonWorker(thread_pool_size);
  ret &= StartTimerWorker();
  return ret;
}

int App::LoadServiceFromDir(const std::string& path) {
  auto service_list = Common::GetDirFiles(path);
  LOG(INFO)
    << "Search " << service_list.size() << " service conf"
    << ", from " << path;
  if (service_list.empty()) {
    LOG(WARNING)
      << "Can't find service conf file,"
      << " skip load from service conf file";
    return -1;
  }
  int load_service_cnt = 0;
  for (const auto& it : service_list) {
    if (!LoadServiceFromFile(it.string())) {
      LOG(ERROR) << "Load " << it.string() << " failed";
      return -1;
    }
    load_service_cnt++;
  }
  return load_service_cnt;
}

bool App::LoadServiceFromFile(const std::string& file) {
  LOG(INFO) << "Load service from " << file << " ...";
  auto root = Common::LoadJsonFromFile(file);
  return LoadServiceFromJson(root);
}

bool App::LoadServiceFromJson(const Json::Value& service) {
  if (state_.load() == State::kUninitialized) {
    LOG(ERROR) << "not init, please call Init() before LoadServiceFromJson()";
    return false;
  }
  if (state_.load() == State::kQuitting
      || state_.load() == State::kQuit) {
    LOG(ERROR) << "program quiting or quit";
    return false;
  }

  // load service
  std::vector<std::pair<Json::Value, std::shared_ptr<Actor>>> actors;
  std::vector<std::pair<Json::Value, std::shared_ptr<Worker>>> workers;
  if (!mods_->LoadService(lib_dir_, service, &actors, &workers)) {
    LOG(ERROR) << "load service failed " << service.toStyledString();
    return false;
  }

  // add actor
  for (size_t i = 0; i < actors.size(); ++i) {
    const auto& p = actors[i];
    const auto& inst = p.first;
    auto actor = p.second;

    Json::Value cfg;
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    if (!AddActor(actor, cfg)) {
      return false;
    }
  }

  // add worker
  for (size_t i = 0; i < workers.size(); ++i) {
    const auto& p = workers[i];
    const auto& inst = p.first;
    auto worker = p.second;

    Json::Value cfg;
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    if (!AddWorker(worker, cfg)) {
      return false;
    }
  }

  return true;
}

bool App::AddActor(
  std::shared_ptr<Actor> actor,
  const Json::Value& config) {
  if (state_.load() == State::kUninitialized) {
    LOG(ERROR) << "not init, please call Init() before AddActor()";
    return false;
  }
  if (state_.load() == State::kQuitting
      || state_.load() == State::kQuit) {
    LOG(ERROR) << "program quiting or quit";
    return false;
  }

  auto actor_name = actor->GetActorName();
  if (actor->GetTypeName() == "node") {
    std::lock_guard<std::recursive_mutex> lock(local_mtx_);
    if (node_addr_.empty()) {
      LOG(INFO) << "create node " << actor_name;
      node_addr_ = actor_name;
    } else {
      LOG(ERROR) << "has more than one node instance, "
        << node_addr_ << " and " << actor_name;
      return false;
    }
  }
  auto ctx = std::make_shared<ActorContext>(shared_from_this(), actor);
  if (ctx->Init(config)) {
    LOG(ERROR) << "init " << actor_name << " fail";
    return false;
  }
  actor_ctx_mgr_->Add(ctx);
  std::lock_guard<std::recursive_mutex> lock(local_mtx_);
  // 接收缓存中发给自己的消息
  for (auto it = cache_msgs_.begin(); it != cache_msgs_.end();) {
    if ((*it)->GetDst() == actor_name) {
      LOG(INFO) << actor_name
        << " recv msg from cache " << *(it);
      DispatchMsg(*it);
      it = cache_msgs_.erase(it);
      continue;
    }
    ++it;
  }
  // 目的地址不存在的暂时放到缓存消息队列
  // 在运行时不再缓存，直接分发
  if (state_.load() != State::kRunning) {
    auto send_list = ctx->GetMailbox()->GetSendList();
    for (auto it = send_list->begin(); it != send_list->end();) {
      if (!HasUserInst((*it)->GetDst())
          || (*it)->GetTransMode() == Msg::TransMode::kDDS
          || (*it)->GetTransMode() == Msg::TransMode::kHybrid) {
        LOG(WARNING) << "can't found " << (*it)->GetDst()
          << ", cache this msg";
        cache_msgs_.push_back(*it);
        it = send_list->erase(it);
        continue;
      }
      ++it;
    }
  }
  // 分发目的地址已经存在的消息
  DispatchMsg(ctx);
  return true;
}

bool App::AddWorker(
  std::shared_ptr<Worker> worker,
  const Json::Value& config) {
  if (state_.load() == State::kUninitialized) {
    LOG(ERROR) << "not init, please call Init() before AddWorker()";
    return false;
  }
  if (state_.load() == State::kQuitting
      || state_.load() == State::kQuit) {
    LOG(ERROR) << "program quiting or quit";
    return false;
  }
  auto worker_ctx = std::make_shared<WorkerContext>(
    shared_from_this(), worker, poller_);
  if (!worker_ctx->Init(config)) {
    LOG(ERROR) << "worker context init failed";
    return false;
  }

  if (worker->GetTypeName() == "node") {
    std::lock_guard<std::recursive_mutex> lock(local_mtx_);
    if (node_addr_.empty()) {
      LOG(INFO) << "create node " << worker->GetWorkerName();
      node_addr_ = worker->GetWorkerName();
    } else {
      LOG(ERROR) << "has more than one node instance, "
        << node_addr_ << " and " << worker->GetWorkerName();
      return false;
    }
  }
  if (!worker_ctx_mgr_->Add(worker_ctx)) {
    return false;
  }
  if (!poller_->Add(worker_ctx)) {
    return false;
  }
  worker_ctx->Start();
  return true;
}

int App::Send(std::shared_ptr<Msg> msg) {
  if (state_.load() != State::kRunning) {
    VLOG(1) << "program not runing";
    return -1;
  }
  auto conn = ev_conn_mgr_->Alloc();
  if (conn == nullptr) {
    LOG(ERROR) << "alloc conn event failed";
    return -1;
  }
  poller_->Add(conn);
  auto ret = conn->Send(std::move(msg));
  poller_->Del(conn);
  // 主动释放
  ev_conn_mgr_->Release(std::move(conn));
  return ret;
}

const std::shared_ptr<const Msg> App::SendRequest(
  std::shared_ptr<Msg> msg) {
  if (state_.load() != State::kRunning) {
    VLOG(1) << "program not runing";
    return nullptr;
  }
  auto conn = ev_conn_mgr_->Alloc();
  if (conn == nullptr) {
    LOG(ERROR) << "alloc conn event failed";
    return nullptr;
  }
  poller_->Add(conn);
  auto resp = conn->SendRequest(std::move(msg));
  poller_->Del(conn);
  ev_conn_mgr_->Release(std::move(conn));
  return resp;
}

std::unique_ptr<ModManager>& App::GetModManager() {
  return mods_;
}

bool App::StartCommonWorker(int worker_count) {
  bool ret = false;
  for (int i = 0; i < worker_count; ++i) {
    auto worker = std::make_shared<WorkerCommon>();
    worker->SetModName("class");
    worker->SetTypeName("C");
    worker->SetInstName(std::to_string(i));
    if (!AddWorker(worker)) {
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
  worker->SetTypeName("T");
  worker->SetInstName("1");
  if (!AddWorker(worker)) {
    LOG(ERROR) << "start timer worker failed";
    return false;
  }
  LOG(INFO) << "start timer worker " << worker->GetWorkerName();
  return true;
}

void App::DispatchMsg(std::shared_ptr<Msg> msg) {
  std::lock_guard<std::recursive_mutex> lock(local_mtx_);
  VLOG(1) << *msg;
  /// 消息分发
  Common::SplitMsgName(msg->GetDst(), &name_list_);
  if (name_list_.size() != 3) {
    LOG(ERROR) << "Unknown msg " << *msg;
    return;
  }
  // trans func
  static auto trans2actor = [this](std::shared_ptr<Msg> m) {
    if (actor_ctx_mgr_->HasActor(m->GetDst())) {
      actor_ctx_mgr_->DispatchMsg(std::move(m));
    }
  };
  static auto trans2worker = [this](std::shared_ptr<Msg> m) {
    if (ev_mgr_->Has(m->GetDst())) {
      worker_ctx_mgr_->DispatchWorkerMsg(std::move(m));
    }
  };
  static auto trans2ev = [this](std::shared_ptr<Msg> m) {
    if (name_list_[1] == "conn") {
      auto handle = ev_mgr_->ToHandle(m->GetDst());
      ev_conn_mgr_->Notify(handle, std::move(m));
    } else {
      LOG(ERROR) << "Unknown msg " << *m;
    }
  };
  static auto trans2dds = [this](std::shared_ptr<Msg> m) {
    if (node_addr_.empty()) {
      return;
    }
    if (node_addr_.substr(0, 5) == "actor") {
      actor_ctx_mgr_->DispatchMsg(m, node_addr_);
    } else if (node_addr_.substr(0, 6) == "worker") {
      worker_ctx_mgr_->DispatchWorkerMsg(m, node_addr_);
    } else {
      LOG(ERROR) << "Unknown msg " << *m;
    }
  };
  static auto trans2hybird1 = [&, this](std::shared_ptr<Msg> m) {
    if (node_addr_.empty()) {
      trans2actor(std::move(m));
      return;
    }
    trans2actor(m);
    trans2dds(std::move(m));
  };
  static auto trans2hybird2 = [&, this](std::shared_ptr<Msg> m) {
    if (node_addr_.empty()) {
      trans2worker(std::move(m));
      return;
    }
    trans2worker(m);
    trans2dds(std::move(m));
  };
  static auto trans2hybird3 = [&, this](std::shared_ptr<Msg> m) {
    if (node_addr_.empty()) {
      trans2ev(std::move(m));
      return;
    }
    trans2ev(m);
    trans2dds(std::move(m));
  };
  // trans vec
  static std::vector<
    std::vector<
      std::function<void(std::shared_ptr<Msg>)>>> trans_map = {
    // kIntra, kDDS, kHybird
    {trans2actor, trans2dds, trans2hybird1},  // actor
    {trans2worker, trans2dds, trans2hybird2},  // worker
    {trans2ev, trans2dds, trans2hybird3}  // connevent
  };
  // trans
  int trans_idx1 = -1;
  int trans_idx2 = -1;
  if (name_list_[0] == "actor") {
    trans_idx1 = 0;
  } else if (name_list_[0] == "worker") {
    trans_idx1 = 1;
  } else if (name_list_[0] == "event") {
    trans_idx1 = 2;
  }
  auto trans_mode = msg->GetTransMode();
  if (trans_mode == Msg::TransMode::kIntra) {
    trans_idx2 = 0;
  } else if (trans_mode == Msg::TransMode::kDDS) {
    trans_idx2 = 1;
  } else if (trans_mode == Msg::TransMode::kHybrid) {
    trans_idx2 = 2;
  }
  if (trans_idx1 == -1 || trans_idx2 == -1) {
    LOG(ERROR) << "Unknown msg " << *msg;
    return;
  }
  trans_map[trans_idx1][trans_idx2](std::move(msg));
}

void App::DispatchMsg(std::list<std::shared_ptr<Msg>>* msg_list) {
  for (auto& msg : (*msg_list)) {
    DispatchMsg(std::move(msg));
  }
  msg_list->clear();
}

// 将获得的消息分发给其他actor
void App::DispatchMsg(std::shared_ptr<ActorContext> context) {
  if (nullptr == context) {
    return;
  }
  VLOG(1) << context->GetActor()->GetActorName() << " dispatch msg...";
  context->SetRuningFlag(false);
  auto msg_list = context->GetMailbox()->GetSendList();
  DispatchMsg(msg_list);
}

void App::CheckStopWorkers() {
  VLOG(1) << "check stop worker";
  worker_ctx_mgr_->WeakupWorker();

  LOG_IF(WARNING, worker_ctx_mgr_->IdleWorkerSize() == 0)
      << "worker busy, wait for idle worker...";
  std::shared_ptr<ActorContext> actor_ctx = nullptr;
  std::shared_ptr<WorkerContext> worker_ctx = nullptr;
  while ((worker_ctx = worker_ctx_mgr_->FrontIdleWorker()) != nullptr) {
    if (nullptr == (actor_ctx = actor_ctx_mgr_->GetContextWithMsg())) {
      VLOG(1) << "no actor need process, waiting...";
      break;
    }
    VLOG(1)
      << actor_ctx->GetActor()->GetActorName()
      << " dispatch msg to "
      << *worker_ctx;
    auto actor_mailbox = actor_ctx->GetMailbox();
    std::size_t actor_ctx_recv_sz = actor_mailbox->RecvSize();
    if (!actor_mailbox->RecvEmpty()) {
      LOG_IF(WARNING,
        actor_ctx_recv_sz > warning_msg_size_.load())
          << actor_ctx->GetActor()->GetActorName()
          << " recv msg size too many: " << actor_ctx_recv_sz;
      VLOG(1) << "run " << actor_ctx->GetActor()->GetActorName();
      actor_mailbox->MoveToRun();
      VLOG(1) << actor_ctx->GetActor()->GetActorName()
        << " has " << actor_ctx_recv_sz
        << " msg need process";
      worker_ctx_mgr_->PopFrontIdleWorker();
      auto common_idle_worker = worker_ctx->GetWorker<WorkerCommon>();
      common_idle_worker->SetActorContext(actor_ctx);
      // 接收队列不空，重新加入等待执行队列
      if (!actor_mailbox->RecvEmpty()) {
        actor_ctx_mgr_->PushContext(std::move(actor_ctx));
      }
      worker_ctx->GetCmdChannel()->SendToOwner(CmdChannel::Cmd::kRun);
    } else {
      LOG(ERROR) << actor_ctx->GetActor()->GetActorName() << " has no msg";
    }
  }
}

std::vector<std::string> App::GetAllUserModAddr() const {
  auto res_actor = actor_ctx_mgr_->GetAllActorAddr();
  auto res_worker = worker_ctx_mgr_->GetAllUserWorkerAddr();
  std::vector<std::string> addr_list;
  addr_list.reserve(res_actor.size() + res_worker.size());
  addr_list.insert(addr_list.end(), res_actor.begin(), res_actor.end());
  addr_list.insert(addr_list.end(), res_worker.begin(), res_worker.end());
  return addr_list;
}

void App::ProcessTimerEvent(std::shared_ptr<WorkerContext> worker_ctx) {
  // 将定时器线程的发送队列分发完毕
  VLOG(1) << *worker_ctx << " dispatch msg...";
  DispatchMsg(worker_ctx->GetMailbox()->GetSendList());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker_ctx->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      VLOG(1) << *worker_ctx << " run again";
      cmd_channel->SendToOwner(CmdChannel::Cmd::kRun);
      break;
    case CmdChannel::Cmd::kQuit:  // quit
      LOG(INFO) << *worker_ctx
                << " quit, delete from main";
      poller_->Del(worker_ctx);
      worker_ctx_mgr_->Del(worker_ctx);
      break;
    default:
      LOG(WARNING) << "Unknown timer worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

void App::ProcessUserEvent(std::shared_ptr<WorkerContext> worker_ctx) {
  // 将用户线程的发送队列分发完毕
  VLOG(1) << *worker_ctx << " dispatch msg...";
  DispatchMsg(worker_ctx->GetMailbox()->GetSendList());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker_ctx->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      VLOG(1) << *worker_ctx << " run again";
      cmd_channel->SendToOwner(CmdChannel::Cmd::kRun);
      break;
    case CmdChannel::Cmd::kWaitForMsg:
      VLOG(1) << *worker_ctx << " wait for msg...";
      worker_ctx_mgr_->PushWaitWorker(worker_ctx);
      break;
    case CmdChannel::Cmd::kQuit:  // quit
      LOG(INFO) << *worker_ctx << " quit, delete from main";
      poller_->Del(worker_ctx);
      worker_ctx_mgr_->Del(worker_ctx);
      break;
    default:
      LOG(WARNING) << "Unknown user worker cmd: " << static_cast<char>(cmd);
      break;
  }
}

/// FIXME: Idle/DispatchMsg 会影响actor的执行顺序
void App::ProcessWorkerEvent(std::shared_ptr<WorkerContext> worker_ctx) {
  // 将actor的发送队列分发完毕
  auto worker = worker_ctx->GetWorker<WorkerCommon>();
  VLOG_IF(1, worker->GetActorContext() != nullptr)
      << *worker_ctx << " dispatch "
      << worker->GetActorContext()->GetActor()->GetActorName() << " msg...";
  DispatchMsg(worker->GetActorContext());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      // 将工作线程中的actor状态设置为全局状态
      // 将线程加入空闲队列
      VLOG(1)
        << *worker_ctx
        << " idle, push to idle queue";
      worker->Idle();
      worker_ctx_mgr_->PushBackIdleWorker(worker_ctx);
      break;
    case CmdChannel::Cmd::kQuit:  // quit
      LOG(INFO)
        << *worker_ctx
        << " quit, delete from main";
      poller_->Del(worker_ctx);
      worker_ctx_mgr_->Del(worker_ctx);
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
  auto cmd_channel = ev->GetCmdChannel();
  CmdChannel::Cmd cmd;
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kRun:
      cmd_channel->SendToOwner(CmdChannel::Cmd::kIdle);
      break;
    case CmdChannel::Cmd::kRunWithMsg:
      // do nothing
      break;
    default:
      LOG(WARNING) << "unknown cmd: " << static_cast<char>(cmd);
      break;
  }
}

void App::ProcessEvent(const std::vector<ev_handle_t>& evs) {
  VLOG(1) << "get " << evs.size() << " event";
  for (size_t i = 0; i < evs.size(); ++i) {
    auto ev_obj = ev_mgr_->Get<Event>(evs[i]);
    if (ev_obj == nullptr) {
      std::stringstream ss;
      for (size_t x = 0; x < evs.size(); ++x) {
        ss << evs[x] << ", ";
      }
      LOG(WARNING) << "get evs " << ss.str();
      LOG(WARNING) << "can't find ev obj, handle " << evs[i];
      continue;
    }
    switch (ev_obj->GetType()) {
      case Event::Type::kWorkerCommon:
        ProcessWorkerEvent(std::dynamic_pointer_cast<WorkerContext>(ev_obj));
        break;
      case Event::Type::kWorkerTimer:
        ProcessTimerEvent(std::dynamic_pointer_cast<WorkerContext>(ev_obj));
        break;
      case Event::Type::kWorkerUser:
        ProcessUserEvent(std::dynamic_pointer_cast<WorkerContext>(ev_obj));
        break;
      case Event::Type::kEventConn:
        ProcessEventConn(std::dynamic_pointer_cast<EventConn>(ev_obj));
        break;
      default:
        LOG(WARNING) << "unknown event";
        break;
    }
  }
}

int App::Exec() {
  // 已经初始化或者加载service失败正在退出才能执行Exec函数
  if (state_.load() != State::kInitialized
      && state_.load() != State::kQuitting) {
    LOG(ERROR) << "not init or quiting";
    return -1;
  }
  int time_wait_ms = 100;
  std::vector<ev_handle_t> evs;
  state_.store(State::kRunning);
  /// 处理初始化中缓存消息
  DispatchMsg(&cache_msgs_);
  while (worker_ctx_mgr_->WorkerSize()) {
    /// 检查空闲线程队列是否有空闲线程，如果有就找到一个有消息的actor处理
    CheckStopWorkers();
    /// 等待事件
    poller_->Wait(&evs, time_wait_ms);
    /// 处理事件
    ProcessEvent(evs);
  }

  // quit App
  worker_ctx_mgr_->WaitAllWorkerQuit();
  worker_ctx_mgr_->ClearStopWorker();
  ev_conn_mgr_->Clear();
  ev_mgr_->Clear();
  actor_ctx_mgr_->ClearContext();
  state_.store(State::kQuit);
  LOG(INFO) << "app exit exec";
  return 0;
}

void App::Quit() {
  // wait worker stop
  if (state_.load() == State::kRunning
      || state_.load() == State::kInitialized) {
    state_.store(State::kQuitting);
    worker_ctx_mgr_->StopAllWorker();
  }
}

bool App::HasUserInst(const std::string& name) {
  if (name.substr(0, 6) == "worker") {
    auto worker_ctx = ev_mgr_->Get<WorkerContext>(name);
    if (worker_ctx != nullptr
        && worker_ctx->GetType() == Event::Type::kWorkerUser) {
      return true;
    }
  } else if (name.substr(0, 5) == "actor") {
    if (actor_ctx_mgr_->HasActor(name)) {
      return true;
    }
  }
  return false;
}

int App::GetDefaultPendingQueueSize() const {
  return default_pending_queue_size_;
}

int App::GetDefaultRunQueueSize() const {
  return default_run_queue_size_;
}

App::State App::GetState() const {
  return state_.load();
}

}  // namespace myframe
