/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/worker.h"

#include <functional>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/worker_context.h"
#include "myframe/app.h"

namespace myframe {

Worker::~Worker() {}

const std::string Worker::GetWorkerName() const {
  return "worker." + worker_name_ + "." + inst_name_;
}
const std::string& Worker::GetModName() const { return mod_name_; }
const std::string& Worker::GetTypeName() const { return worker_name_; }
const std::string& Worker::GetInstName() const { return inst_name_; }
void Worker::SetModName(const std::string& name) { mod_name_ = name; }
void Worker::SetTypeName(const std::string& name) { worker_name_ = name; }
void Worker::SetInstName(const std::string& name) { inst_name_ = name; }

void Worker::Stop() {
  if (ctx_ == nullptr) {
    return;
  }
  ctx_->Stop();
}

int Worker::DispatchMsg() {
  auto channel = GetCmdChannel();
  CmdChannel::Cmd cmd = CmdChannel::Cmd::kIdle;
  channel->SendToMain(cmd);
  auto ret = channel->RecvFromMain(&cmd);
  if (cmd == CmdChannel::Cmd::kQuit) {
    LOG(INFO) << GetWorkerName() << " recv stop msg, stoping...";
    Stop();
    return -1;
  }
  return ret;
}

int Worker::DispatchAndWaitMsg() {
  auto channel = GetCmdChannel();
  CmdChannel::Cmd cmd = CmdChannel::Cmd::kWaitForMsg;
  channel->SendToMain(cmd);
  auto ret = channel->RecvFromMain(&cmd);
  if (cmd == CmdChannel::Cmd::kQuit) {
    LOG(INFO) << GetWorkerName() << " recv stop msg, stoping...";
    Stop();
    return -1;
  }
  return ret;
}

Mailbox* Worker::GetMailbox() {
  if (ctx_ == nullptr) {
    return nullptr;
  }
  return ctx_->GetMailbox();
}

CmdChannel* Worker::GetCmdChannel() {
  LOG_IF(FATAL, ctx_ == nullptr)
    << "worker ctx is nullptr";
  return ctx_->GetCmdChannel();
}

void Worker::SetConfig(const Json::Value& conf) {
  config_ = conf;
}

const Json::Value* Worker::GetConfig() const {
  return &config_;
}

void Worker::SetContext(WorkerContext* ctx) {
  ctx_ = ctx;
}

Event::Type Worker::GetType() {
  return Event::Type::kWorkerUser;
}

std::shared_ptr<App> Worker::GetApp() {
  if (ctx_ == nullptr) {
    return nullptr;
  }
  return ctx_->GetApp();
}

}  // namespace myframe
