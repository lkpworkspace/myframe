/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "myframe/worker.h"

#include <functional>

#include <glog/logging.h>

#include "myframe/common.h"
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
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return;
  }
  ctx->Stop();
}

int Worker::DispatchMsg() {
  auto channel = GetCmdChannel();
  if (channel == nullptr) {
    return -1;
  }
  Cmd cmd = Cmd::kIdle;
  channel->SendToMain(cmd);
  return channel->RecvFromMain(&cmd);
}

int Worker::DispatchAndWaitMsg() {
  auto channel = GetCmdChannel();
  if (channel == nullptr) {
    return -1;
  }
  Cmd cmd = Cmd::kWaitForMsg;
  channel->SendToMain(cmd);
  return channel->RecvFromMain(&cmd);
}

Mailbox* Worker::GetMailbox() {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return nullptr;
  }
  return ctx->GetMailbox();
}

CmdChannel* Worker::GetCmdChannel() {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return nullptr;
  }
  return ctx->GetCmdChannel();
}

void Worker::SetConfig(const Json::Value& conf) {
  config_ = conf;
}

const Json::Value* Worker::GetConfig() const {
  return &config_;
}

void Worker::SetContext(std::shared_ptr<WorkerContext> ctx) {
  ctx_ = ctx;
}

EventType Worker::GetType() {
  return EventType::kWorkerUser;
}

std::shared_ptr<App> Worker::GetApp() {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return nullptr;
  }
  return ctx->GetApp();
}

}  // namespace myframe
