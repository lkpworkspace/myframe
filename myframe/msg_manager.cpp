/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/msg_manager.h"
#include "myframe/log.h"

namespace myframe {
MsgManager::MsgManager() {
  VLOG(1) << "MsgManager create";
}

MsgManager::~MsgManager() {
  VLOG(1) << "MsgManager deconstruct";
}

bool MsgManager::AddSubInfo(const std::shared_ptr<Msg>& msg) {
  auto topic_name = GetTopicName(msg, true);
  auto tm = msg->GetTransMode();
  if (tm == Msg::TransMode::kIntra || tm == Msg::TransMode::kHybrid) {
    // 进程间通信的订阅消息，需要记录订阅信息
    SubInfo sub_info;
    sub_info.com_name = msg->GetSrc();
    sub_info.msg_name = msg->GetName();
    auto it = sub_list_.find(topic_name);
    if (it == sub_list_.end()) {
      // 订阅topic不存在就添加
      sub_list_[topic_name].emplace(sub_info);
      LOG(INFO) << "record sub msg: " << *msg;
    } else {
      auto& sub_info_set = it->second;
      if (sub_info_set.find(SubInfo(sub_info.com_name)) ==
          sub_info_set.end()) {
        // 订阅topic存在，但是订阅组件不存在就添加
        sub_list_[topic_name].emplace(sub_info);
        LOG(INFO) << "record sub msg: " << *msg;
      } else {
        // 订阅topic存在，订阅组件也存在
        // 说明是重复订阅，此处需要告警
        LOG(WARNING) << "repeat sub msg: " << *msg << " ignore!";
        return false;
      }
    }
  }
  // 如果是进程间/机器间订阅消息，需要把订阅消息转发给node节点
  return ((tm == Msg::TransMode::kDDS) || (tm == Msg::TransMode::kHybrid));
}

bool MsgManager::AddPubInfo(const std::shared_ptr<Msg>& msg) {
  auto topic_name = GetTopicName(msg, false);
  auto tm = msg->GetTransMode();
  if (tm == Msg::TransMode::kIntra) {
    // 只有在进程内通信并且找不到订阅者的情况下才会丢弃消息
    auto it = sub_list_.find(topic_name);
    if (it == sub_list_.end()) {
      return false;
    }
  }
  return true;
}

void MsgManager::DispatchPubMsg(
    std::shared_ptr<Msg> msg,
    pub_cb_t pub_cb) {
  auto topic_name = GetTopicName(msg, false);
  auto tm = msg->GetTransMode();
  if ((tm == Msg::TransMode::kIntra) || (tm == Msg::TransMode::kHybrid)) {
    // 将消息分发给进程内订阅者
    auto sub_info_list = sub_list_[topic_name];
    auto it = sub_info_list.begin();
    for (; it != sub_info_list.end(); ++it) {
      pub_cb(msg, it->com_name, Msg::TransMode::kIntra);
    }
  }
  if ((tm == Msg::TransMode::kDDS) || (tm == Msg::TransMode::kHybrid)) {
    // 将消息分发给node节点
    pub_cb(msg, "", Msg::TransMode::kDDS);
  }
}

std::string MsgManager::GetTopicName(
    const std::shared_ptr<Msg>& msg,
    bool sub_msg) {
  std::string topic_name;
  if (sub_msg) {
    topic_name = msg->GetDst();
  } else {
    topic_name = msg->GetSrc();
  }
  if (topic_name.empty()) {
    return "";
  }
  if (!msg->GetName().empty()) {
    topic_name += '.';
    topic_name += msg->GetName();
  }
  return topic_name;
}

}  // namespace myframe
