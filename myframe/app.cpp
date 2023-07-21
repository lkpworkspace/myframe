/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/app.h"

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/mailbox.h"
#include "myframe/actor.h"
#include "myframe/actor_context.h"
#include "myframe/actor_context_manager.h"
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
  std::string worker_timer_name = "worker.timer.#1";
  auto w = worker_ctx_mgr_->Get(worker_timer_name);
  if (w == nullptr) {
    LOG(ERROR)
      << "can't find " << worker_timer_name;
    return nullptr;
  }
  auto timer_worker = w->GetWorker<WorkerTimer>();
  return timer_worker;
}

App::App()
  : poller_(new Poller())
  , mods_(new ModManager())
  , actor_ctx_mgr_(new ActorContextManager())
  , ev_conn_mgr_(new EventConnManager())
  , worker_ctx_mgr_(new WorkerContextManager())
{}

App::~App() {
  LOG(INFO) << "app deconstruct";
}

bool App::Init(
  const std::string& lib_dir,
  int thread_pool_size,
  int event_conn_size,
  int warning_msg_size) {
  if (!quit_.load()) {
    return true;
  }

  bool ret = true;
  lib_dir_ = lib_dir;
  warning_msg_size_.store(warning_msg_size);
  ret &= poller_->Init();
  ret &= worker_ctx_mgr_->Init(warning_msg_size);
  ret &= ev_conn_mgr_->Init(shared_from_this(), event_conn_size);
  ret &= StartCommonWorker(thread_pool_size);
  ret &= StartTimerWorker();

  quit_.store(false);
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
    return false;
  }
  int load_service_cnt = 0;
  for (const auto& it : service_list) {
    if (!LoadServiceFromFile(it)) {
      LOG(ERROR) << "Load " << it << " failed";
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

bool App::LoadServiceFromJsonStr(const std::string& service) {
  Json::Value root;
  Json::Reader reader(Json::Features::strictMode());
  if (!reader.parse(service, root)) {
    LOG(ERROR) << "parse service string failed";
    return false;
  }
  return LoadServiceFromJson(root);
}

bool App::LoadServiceFromJson(const Json::Value& service) {
  if (service.isNull()) {
    LOG(ERROR) << "parse service json failed, skip";
    return false;
  }
  if (!service.isMember("type") || !service["type"].isString()) {
    LOG(ERROR) << "key \"type\": no key or not string, skip";
    return false;
  }
  const auto& type = service["type"].asString();
  std::string lib_name;
  // load library
  if (type == "library") {
    if (!service.isMember("lib") || !service["lib"].isString()) {
      LOG(ERROR) << " key \"lib\": no key or not string, skip";
      return false;
    }
    lib_name = service["lib"].asString();
    auto lib_dir = Common::GetAbsolutePath(lib_dir_);
    if (!mods_->LoadMod(lib_dir + lib_name)) {
      LOG(ERROR) << "load lib " << (lib_dir + lib_name) << " failed, skip";
      return false;
    }
  }
  bool res = true;
  // load actor
  if (service.isMember("actor") && service["actor"].isObject()) {
    const auto& actor_list = service["actor"];
    Json::Value::Members actor_name_list = actor_list.getMemberNames();
    for (auto inst_name_it = actor_name_list.begin();
          inst_name_it != actor_name_list.end(); ++inst_name_it) {
      LOG(INFO) << "search actor " << *inst_name_it << " ...";
      if (type == "library") {
        res = LoadActors(lib_name, *inst_name_it, actor_list);
      } else if (type == "class") {
        res = LoadActors("class", *inst_name_it, actor_list);
      } else {
        LOG(ERROR) << "Unknown type " << type;
        res = false;
      }
      if (!res) {
        return res;
      }
    }
  }
  // load worker
  if (service.isMember("worker") && service["worker"].isObject()) {
    const auto& worker_list = service["worker"];
    Json::Value::Members worker_name_list = worker_list.getMemberNames();
    for (auto inst_name_it = worker_name_list.begin();
          inst_name_it != worker_name_list.end(); ++inst_name_it) {
      LOG(INFO) << "search worker " << *inst_name_it << " ...";
      if (type == "library") {
        res = LoadWorkers(lib_name, *inst_name_it, worker_list);
      } else if (type == "class") {
        res = LoadWorkers("class", *inst_name_it, worker_list);
      } else {
        LOG(ERROR) << "Unknown type " << type;
        res = false;
      }
      if (!res) {
        return res;
      }
    }  // end for
  }  // end load worker
  return res;
}

bool App::LoadActors(
  const std::string& mod_name,
  const std::string& actor_name,
  const Json::Value& actor_list) {
  const auto& insts = actor_list[actor_name];
  for (const auto& inst : insts) {
    std::string inst_name;
    std::string inst_param;
    Json::Value cfg;
    LOG(INFO)
      << "create actor instance \"" << actor_name
      << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR)
        << "actor " << actor_name
        << " key \"instance_name\": no key, skip";
      return false;
    }
    inst_name = inst["instance_name"].asString();
    if (inst.isMember("instance_params")) {
      inst_param = inst["instance_params"].asString();
    }
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    auto res = CreateActorContext(
      mod_name,
      actor_name,
      inst_name,
      inst_param,
      cfg);
    if (!res) {
      return res;
    }
  }
  return true;
}

bool App::LoadWorkers(
  const std::string& mod_name,
  const std::string& worker_name,
  const Json::Value& worker_list) {
  const auto& insts = worker_list[worker_name];
  for (const auto& inst : insts) {
    std::string inst_name;
    Json::Value cfg;
    LOG(INFO)
      << "create worker instance \"" << worker_name
      << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR)
        << "worker " << worker_name
        << " key \"instance_name\": no key, skip";
      return false;
    }
    inst_name = inst["instance_name"].asString();
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    auto worker = mods_->CreateWorkerInst(mod_name, worker_name);
    if (worker == nullptr) {
      LOG(ERROR)
        << "create worker " << mod_name << "." << worker_name << "."
        << inst_name << " failed, continue";
      return false;
    }
    if (!AddWorker(inst_name, worker, cfg)) {
      return false;
    }
  }
  return true;
}

bool App::AddActor(
  const std::string& inst_name,
  const std::string& params,
  std::shared_ptr<Actor> actor,
  const Json::Value& config) {
  actor->SetInstName(inst_name);
  actor->SetConfig(config);
  return CreateActorContext(actor, params);
}

bool App::AddWorker(
  const std::string& inst_name,
  std::shared_ptr<Worker> worker,
  const Json::Value& config) {
  auto worker_ctx = std::make_shared<WorkerContext>(
    shared_from_this(), worker);
  worker->SetContext(worker_ctx);
  worker->SetInstName(inst_name);
  worker->SetConfig(config);
  if (worker->GetTypeName() == "node") {
    std::lock_guard<std::mutex> lock(local_mtx_);
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

int App::Send(
  const std::string& dst,
  std::shared_ptr<Msg> msg) {
  auto conn = ev_conn_mgr_->Get();
  auto ret = conn->Send(dst, msg);
  ev_conn_mgr_->Release(conn);
  return ret;
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
bool App::CreateActorContext(
  const std::string& mod_name,
  const std::string& actor_name,
  const std::string& instance_name,
  const std::string& params,
  const Json::Value& config) {
  auto actor_inst = mods_->CreateActorInst(mod_name, actor_name);
  if (actor_inst == nullptr) {
    LOG(ERROR) << "Create mod " << mod_name << "." << actor_name << " failed";
    return false;
  }
  actor_inst->SetInstName(instance_name);
  actor_inst->SetConfig(config);
  return CreateActorContext(actor_inst, params);
}

bool App::CreateActorContext(
  std::shared_ptr<Actor> mod_inst,
  const std::string& params) {
  auto ctx = std::make_shared<ActorContext>(shared_from_this(), mod_inst);
  if (ctx->Init(params.c_str())) {
    LOG(ERROR) << "init " << mod_inst->GetActorName() << " fail";
    return false;
  }
  actor_ctx_mgr_->RegContext(ctx);
  DispatchMsg(ctx);
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
      msg_list->size() > warning_msg_size_.load())
    << " dispatch msg too many";
  std::string node_addr;
  {
    std::lock_guard<std::mutex> lock(local_mtx_);
    node_addr = node_addr_;
  }
  std::lock_guard<std::mutex> lock(dispatch_mtx_);
  for (auto& msg : (*msg_list)) {
    DLOG(INFO) << *msg;
    /// 处理框架消息
    if (msg->GetDst() == MAIN_ADDR) {
      ProcessMain(msg);
      continue;
    }
    /// 转换来自其它框架的消息
    if (!node_addr.empty() && msg->GetSrc() == node_addr) {
      // FIXME: 不支持进程间/机器间转发event.conn消息
      if (msg->GetDesc().substr(0, 10) == "event.conn") {
        continue;
      }
      msg->SetSrc(msg->GetDesc());
      msg->SetDesc(node_addr);
    }
    /// 消息分发
    auto name_list = Common::SplitMsgName(msg->GetDst());
    if (name_list.size() < 2) {
      LOG(ERROR) << "Unknown msg " << *msg;
      continue;
    }
    if (name_list[0] == "worker"
        && worker_ctx_mgr_->HasWorker(msg->GetDst())) {
      // dispatch to user worker
      worker_ctx_mgr_->DispatchWorkerMsg(msg);
    } else if (name_list[0] == "actor"
               && actor_ctx_mgr_->HasActor(msg->GetDst())) {
      // dispatch to actor
      actor_ctx_mgr_->DispatchMsg(msg);
    } else if (name_list[0] == "event") {
      if (name_list[1] == "conn") {
        ev_conn_mgr_->Notify(msg->GetDst(), msg);
      } else {
        LOG(ERROR) << "Unknown msg " << *msg;
      }
    } else {
      if (!node_addr.empty()) {
        msg->SetDesc(msg->GetDst());
        msg->SetDst(node_addr);
        worker_ctx_mgr_->DispatchWorkerMsg(msg);
      } else {
        LOG(ERROR) << "Unknown msg " << *msg;
      }
    }
  }
  msg_list->clear();
}

// 将获得的消息分发给其他actor
void App::DispatchMsg(std::shared_ptr<ActorContext> context) {
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
  worker_ctx_mgr_->WeakupWorker();

  LOG_IF(INFO, worker_ctx_mgr_->IdleWorkerSize() == 0)
      << "worker busy, wait for idle worker...";
  std::shared_ptr<ActorContext> actor_ctx = nullptr;
  std::shared_ptr<WorkerContext> worker_ctx = nullptr;
  while ((worker_ctx = worker_ctx_mgr_->FrontIdleWorker()) != nullptr) {
    if (nullptr == (actor_ctx = actor_ctx_mgr_->GetContextWithMsg())) {
      DLOG(INFO) << "no actor need process, waiting...";
      break;
    }
    DLOG(INFO)
      << actor_ctx->GetActor()->GetActorName()
      << " dispatch msg to "
      << *worker_ctx;
    auto msg_list = actor_ctx->GetMailbox()->GetRecvList();
    if (!msg_list->empty()) {
      LOG_IF(WARNING,
        msg_list->size() > warning_msg_size_.load())
          << actor_ctx->GetActor()->GetActorName()
          << " recv msg size too many: " << msg_list->size();
      DLOG(INFO) << "run " << actor_ctx->GetActor()->GetActorName();
      worker_ctx->GetMailbox()->Recv(msg_list);
      DLOG(INFO) << actor_ctx->GetActor()->GetActorName()
        << " has " << worker_ctx->GetMailbox()->RecvSize()
        << " msg need process";
      worker_ctx_mgr_->PopFrontIdleWorker();
      auto common_idle_worker = worker_ctx->GetWorker<WorkerCommon>();
      common_idle_worker->SetActorContext(actor_ctx);
      worker_ctx->GetCmdChannel()->SendToOwner(CmdChannel::Cmd::kRun);
    } else {
      LOG(ERROR) << actor_ctx->GetActor()->GetActorName() << " has no msg";
    }
  }
}

// Tips: 发送给框架的事件都应该立即处理完成，不应该影响调度
void App::ProcessMain(std::shared_ptr<Msg> msg) {
  // 接收发送给框架消息，处理并回复
  auto src = msg->GetSrc();
  auto cmd = msg->GetData();
  if (cmd.empty()) {
    LOG(WARNING) << "unknown MAIN_CMD " << cmd;
    return;
  }
  auto resp_msg = std::make_shared<Msg>();
  if (cmd == MAIN_CMD_ALL_USER_MOD_ADDR) {
    resp_msg->SetSrc(MAIN_ADDR);
    resp_msg->SetDst(src);
    std::string mod_addr_list;
    GetAllUserModAddr(&mod_addr_list);
    resp_msg->SetData(mod_addr_list);
  } else {
    LOG(WARNING) << "unknown MAIN_CMD " << cmd;
    return;
  }
  if (src.substr(0, 6) == "worker") {
    worker_ctx_mgr_->DispatchWorkerMsg(resp_msg);
  } else if (src.substr(0, 5) == "actor") {
    actor_ctx_mgr_->DispatchMsg(resp_msg);
  } else {
    LOG(ERROR) << "unknow msg " << *msg;
  }
}

void App::GetAllUserModAddr(std::string* info) {
  auto res_actor = actor_ctx_mgr_->GetAllActorAddr();
  auto res_worker = worker_ctx_mgr_->GetAllUserWorkerAddr();
  std::stringstream ss;
  for (std::size_t i = 0; i < res_actor.size(); ++i) {
    ss << res_actor[i] << "\n";
  }
  for (std::size_t i = 0; i < res_worker.size(); ++i) {
    ss << res_worker[i] << "\n";
  }
  info->clear();
  info->append(ss.str());
}

void App::ProcessTimerEvent(std::shared_ptr<WorkerContext> worker_ctx) {
  // 将定时器线程的发送队列分发完毕
  DLOG(INFO) << *worker_ctx << " dispatch msg...";
  DispatchMsg(worker_ctx->GetMailbox()->GetSendList());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker_ctx->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      DLOG(INFO) << *worker_ctx << " run again";
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
  DLOG(INFO) << *worker_ctx << " dispatch msg...";
  DispatchMsg(worker_ctx->GetMailbox()->GetSendList());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker_ctx->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      DLOG(INFO) << *worker_ctx << " run again";
      cmd_channel->SendToOwner(CmdChannel::Cmd::kRun);
      break;
    case CmdChannel::Cmd::kWaitForMsg:
      DLOG(INFO) << *worker_ctx << " wait for msg...";
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
  DLOG_IF(INFO, worker->GetActorContext() != nullptr)
      << *worker_ctx << " dispatch "
      << worker->GetActorContext()->GetActor()->GetActorName() << " msg...";
  LOG_IF(WARNING, worker->GetActorContext() == nullptr)
      << *worker_ctx << " no context";
  DispatchMsg(worker->GetActorContext());

  CmdChannel::Cmd cmd;
  auto cmd_channel = worker->GetCmdChannel();
  cmd_channel->RecvFromOwner(&cmd);
  switch (cmd) {
    case CmdChannel::Cmd::kIdle:  // idle
      // 将工作线程中的actor状态设置为全局状态
      // 将线程加入空闲队列
      DLOG(INFO)
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
  DLOG_IF(INFO, evs.size() > 0) << "get " << evs.size() << " event";
  for (size_t i = 0; i < evs.size(); ++i) {
    std::shared_ptr<Event> ev_obj = nullptr;
    ev_obj = worker_ctx_mgr_->Get(evs[i]);
    if (ev_obj == nullptr) {
      ev_obj = ev_conn_mgr_->Get(evs[i]);
      if (ev_obj == nullptr) {
        LOG(ERROR) << "can't find ev obj, handle " << evs[i];
        continue;
      }
    }
    ev_obj->RetListenIOType(Event::IOType::kIn);
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
  int time_wait_ms = 1000;
  std::vector<ev_handle_t> evs;

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
  quit_.store(true);
  LOG(INFO) << "app exit exec";
  return 0;
}

void App::Quit() {
  // wait worker stop
  if (quit_.load()) {
    return;
  }
  worker_ctx_mgr_->StopAllWorker();
}

}  // namespace myframe
