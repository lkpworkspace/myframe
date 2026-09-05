/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor_context.h"

#include <sstream>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/app.h"
#include "myframe/msg.h"

namespace myframe {

ActorContext::ActorContext(
  std::shared_ptr<App> app,
  std::shared_ptr<Actor> actor)
    : in_worker_(false)
    , in_wait_que_(false)
    , actor_(actor)
    , app_(app) {
  VLOG(1) << this << " create";
}

ActorContext::~ActorContext() {
  VLOG(1) << mailbox_.Addr() << "(" << this << ") context deconstruct";
}

std::shared_ptr<App> ActorContext::GetApp() { return app_.lock(); }

int ActorContext::Init(const Json::Value& conf) {
  actor_->SetContext(this);
  mailbox_.SetAddr(actor_->GetActorName());
  config_ = conf;
  auto app = GetApp();
  int pending_queue_size = app->GetDefaultPendingQueueSize();
  int run_queue_size = app->GetDefaultRunQueueSize();
  if (config_.isMember("pending_queue_size")) {
    pending_queue_size = config_.get("pending_queue_size", -1).asInt();
  }
  if (config_.isMember("run_queue_size")) {
    run_queue_size = config_.get("run_queue_size", -1).asInt();
  }
  mailbox_.SetPendingQueueSize(pending_queue_size);
  mailbox_.SetRunQueueSize(run_queue_size);
  VLOG(1) << mailbox_.Addr() << "(" << this << ") context init";
  return actor_->Init();
}

Mailbox* ActorContext::GetMailbox() {
  return &mailbox_;
}

void ActorContext::Proc(const std::shared_ptr<const Msg>& msg) {
  // FIXME: 需要所有消息都经过Proc处理吗?
  if (msg->GetType() == MYFRAME_MSG_TYPE_RESPONSE) {
    ProcResponse(msg);
  } else {
    actor_->Proc(msg);
  }
}

// 根据请求ID查找回调函数并执行
void ActorContext::ProcResponse(const std::shared_ptr<const Msg>& msg) {
  // LOG(INFO) << mailbox_.Addr() << " request callback size: "
  //   << request_callbacks_.size();
  auto it = request_callbacks_.find(msg->GetDesc());
  if (it == request_callbacks_.end()) {
    LOG(WARNING) << mailbox_.Addr()
      << " request callback not found, request_id: "
      << msg->GetDesc();
    return;
  }
  it->second(msg);
  request_callbacks_.erase(it);
}

bool ActorContext::Request(
    std::shared_ptr<Msg> msg,
    std::function<void(const std::shared_ptr<const Msg>)> callback) {
  if (msg->GetDst().empty()) {
    LOG(WARNING) << mailbox_.Addr() << " request msg dst is empty";
    return false;
  }
  if (msg->GetDst() == mailbox_.Addr()) {
    LOG(WARNING) << mailbox_.Addr() << " request msg dst is self";
    return false;
  }
  // 生成一个唯一的请求ID
  std::string request_id = std::to_string(request_id_++);
  request_callbacks_[request_id] = callback;
  // 发送消息给其它actor
  msg->SetType(MYFRAME_MSG_TYPE_REQUEST);
  msg->SetDesc(request_id);
  mailbox_.Send(msg->GetDst(), std::move(msg));
  return true;
}

bool ActorContext::Response(
    const std::shared_ptr<const Msg>& req_msg,
    std::shared_ptr<Msg> resp_msg) {
  if (req_msg->GetType() != MYFRAME_MSG_TYPE_REQUEST) {
    LOG(WARNING) << mailbox_.Addr() << " request msg type is not request";
    return false;
  }
  if (req_msg->GetDesc().empty()) {
    LOG(WARNING) << mailbox_.Addr() << " request msg request id is empty";
    return false;
  }
  if (req_msg->GetSrc().empty()) {
    LOG(WARNING) << mailbox_.Addr() << " request msg src is empty";
    return false;
  }
  if (req_msg->GetSrc() == mailbox_.Addr()) {
    LOG(WARNING) << mailbox_.Addr() << " request msg src is self";
    return false;
  }
  // 发送消息给其它actor
  resp_msg->SetSrc(mailbox_.Addr());
  resp_msg->SetDst(req_msg->GetSrc());
  resp_msg->SetName(req_msg->GetName());
  resp_msg->SetType(MYFRAME_MSG_TYPE_RESPONSE);
  resp_msg->SetDesc(req_msg->GetDesc());
  mailbox_.Send(std::move(resp_msg));
  return true;
}

const Json::Value* ActorContext::GetConfig() const {
  return &config_;
}

std::ostream& operator<<(std::ostream& out, const ActorContext& ctx) {
  out << ctx.actor_->GetActorName() << ", in worker: " << ctx.in_worker_
     << ", in wait queue: " << ctx.in_wait_que_;
  return out;
}

}  // namespace myframe
