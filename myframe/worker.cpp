/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "myframe/common.h"
#include "myframe/log.h"

namespace myframe {

Worker::Worker() : posix_thread_id_(-1), runing_(false) {
  CreateSockPair();
}

Worker::~Worker() { CloseSockPair(); }

const std::string Worker::GetWorkerName() const {
  return "worker." + worker_name_ + "." + inst_name_;
}

void Worker::Start() {
  int res = 0;
  if (runing_ == false) {
    runing_ = true;
    res =
        pthread_create(&posix_thread_id_, NULL, &Worker::ListenThread, this);
    if (res != 0) {
      runing_ = false;
      LOG(ERROR) << "pthread create failed";
      return;
    }
    res = pthread_detach(posix_thread_id_);
    if (res != 0) {
      runing_ = false;
      LOG(ERROR) << "pthread detach failed";
      return;
    }
  }
}

void Worker::Stop() {
  runing_ = false;
  WorkerCmd cmd = WorkerCmd::QUIT;
  SendCmdToMain(cmd);
}

void* Worker::ListenThread(void* obj) {
  Worker* t = static_cast<Worker*>(obj);
  t->OnInit();
  while (t->runing_) t->Run();
  t->OnExit();
  return nullptr;
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

void Worker::SendMsg(const std::string& dst, std::shared_ptr<Msg> msg) {
  msg->SetSrc(GetWorkerName());
  msg->SetDst(dst);
  send_.emplace_back(msg);
}

void Worker::SendMsg(const std::string& dst, std::any data) {
  auto msg = std::make_shared<Msg>(data);
  SendMsg(dst, msg);
}

void Worker::PushSendMsgList(std::list<std::shared_ptr<Msg>>* msg_list) {
  ListAppend(&send_, msg_list);
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

const std::shared_ptr<const Msg> Worker::GetRecvMsg() {
  if (que_.empty()) {
    return nullptr;
  }
  auto msg = que_.front();
  que_.pop_front();
  return msg;
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
