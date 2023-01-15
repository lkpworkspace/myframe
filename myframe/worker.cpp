/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <functional>

#include <glog/logging.h>

#include "myframe/common.h"

namespace myframe {

Worker::Worker() : runing_(false) {
  CreateSockPair();
}

Worker::~Worker() {
  LOG(INFO) << GetWorkerName() << " deconstruct";
  CloseSockPair();
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
  WorkerCmd cmd = WorkerCmd::QUIT;
  SendCmdToMain(cmd);
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

int Worker::SendCmdToMain(const WorkerCmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  return write(sock_pair_[0], &cmd_char, 1);
}

int Worker::SendCmdToWorker(const WorkerCmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  return write(sock_pair_[1], &cmd_char, 1);
}

int Worker::RecvCmdFromMain(WorkerCmd* cmd, int timeout_ms) {
  if (timeout_ms < 0) {
    // block
    if (!Common::IsBlockFd(sock_pair_[0])) {
      Common::SetNonblockFd(sock_pair_[0], false);
    }
  } else if (timeout_ms == 0) {
    // nonblock
    if (Common::IsBlockFd(sock_pair_[0])) {
      Common::SetNonblockFd(sock_pair_[0], true);
    }
  } else {
    // timeout
    Common::SetSockRecvTimeout(sock_pair_[0], timeout_ms);
  }
  char cmd_char;
  int ret = read(sock_pair_[0], &cmd_char, 1);
  *cmd = (WorkerCmd)cmd_char;
  return ret;
}

int Worker::RecvCmdFromWorker(WorkerCmd* cmd) {
  char cmd_char;
  int ret = read(sock_pair_[1], &cmd_char, 1);
  *cmd = (WorkerCmd)cmd_char;
  return ret;
}

int Worker::DispatchMsg() {
  WorkerCmd cmd = WorkerCmd::IDLE;
  SendCmdToMain(cmd);
  return RecvCmdFromMain(&cmd);
}

int Worker::DispatchAndWaitMsg() {
  WorkerCmd cmd = WorkerCmd::WAIT_FOR_MSG;
  SendCmdToMain(cmd);
  return RecvCmdFromMain(&cmd);
}

bool Worker::CreateSockPair() {
  int res = -1;
  bool ret = true;

  res = socketpair(AF_UNIX, SOCK_DGRAM, 0, sock_pair_);
  if (res == -1) {
    LOG(ERROR) << "Worker create sockpair failed";
    return false;
  }
  ret = Common::SetNonblockFd(sock_pair_[0], false);
  if (!ret) {
    LOG(ERROR) << "Worker set sockpair[0] block failed";
    return ret;
  }
  ret = Common::SetNonblockFd(sock_pair_[1], false);
  if (!ret) {
    LOG(ERROR) << "Worker set sockpair[1] block failed";
    return ret;
  }
  return ret;
}

void Worker::CloseSockPair() {
  if (-1 == close(sock_pair_[0])) {
    LOG(ERROR) << "Worker close sockpair[0]: " << strerror(errno);
  }
  if (-1 == close(sock_pair_[1])) {
    LOG(ERROR) << "Worker close sockpair[1]: " << strerror(errno);
  }
}

}  // namespace myframe
