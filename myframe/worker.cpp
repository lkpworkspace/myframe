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

namespace myframe {

Worker::Worker() : runing_(false) {
}

Worker::~Worker() {
  LOG(INFO) << GetWorkerName() << " deconstruct";
}

int Worker::GetFd() const {
  return cmd_channel_.GetMainFd();
}

const std::string Worker::GetWorkerName() const {
  return "worker." + worker_name_ + "." + inst_name_;
}

const std::string& Worker::GetModName() const { return mod_name_; }
const std::string& Worker::GetTypeName() const { return worker_name_; }
const std::string& Worker::GetInstName() const { return inst_name_; }
void Worker::SetModName(const std::string& name) { mod_name_ = name; }
void Worker::SetTypeName(const std::string& name) { worker_name_ = name; }
void Worker::SetInstName(const std::string& name) { inst_name_ = name; }

void Worker::Start() {
  if (runing_.load() == false) {
    runing_.store(true);
    th_ = std::thread(
        std::bind(&Worker::ListenThread,
                  std::dynamic_pointer_cast<Worker>(shared_from_this())));
    th_.detach();
  }
}

void Worker::Stop() {
  runing_.store(false);
  cmd_channel_.SendToMain(Cmd::kQuit);
}

int Worker::DispatchMsg() {
  Cmd cmd = Cmd::kIdle;
  cmd_channel_.SendToMain(cmd);
  return cmd_channel_.RecvFromMain(&cmd);
}

int Worker::DispatchAndWaitMsg() {
  Cmd cmd = Cmd::kWaitForMsg;
  cmd_channel_.SendToMain(cmd);
  return cmd_channel_.RecvFromMain(&cmd);
}

void Worker::Initialize() {
  mailbox_.SetAddr(GetWorkerName());
  OnInit();
}

void Worker::ListenThread(std::shared_ptr<Worker> w) {
  thread_local std::shared_ptr<Worker> worker_local = w;
  w->Initialize();
  while (w->runing_.load()) {
    w->Run();
  }
  w->OnExit();
}

int Worker::CacheSize() const {
  return cache_.size();
}

std::list<std::shared_ptr<Msg>>* Worker::GetCache() {
  return &cache_;
}

void Worker::Cache(std::shared_ptr<Msg> msg) {
  cache_.emplace_back(msg);
}

void Worker::Cache(std::list<std::shared_ptr<Msg>>* msg_list) {
  ListAppend(&cache_, msg_list);
}

Mailbox* Worker::GetMailbox() {
  return &mailbox_;
}

CmdChannel* Worker::GetCmdChannel() {
  return &cmd_channel_;
}

}  // namespace myframe
