/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "myframe/msg.h"

namespace myframe {

struct SubInfo {
  std::string com_name;
  std::string msg_name;
  std::string msg_desc;

  SubInfo() = default;

  explicit SubInfo(const std::string& cm)
    : com_name(cm) {}

  bool operator==(const SubInfo& o) const {
    if (com_name == o.com_name) {
      return true;
    }
    return false;
  }

  struct Hash {
    size_t operator()(const SubInfo& o) const{
      size_t hash = std::hash<std::string>()(o.com_name);
      return hash;
    }
  };
};

class MsgManager final {
  typedef std::unordered_set<SubInfo, SubInfo::Hash> sub_info_set_t;

 public:
  typedef std::function<
    void(std::shared_ptr<Msg>, const std::string&, Msg::TransMode)> pub_cb_t;

  MsgManager();
  ~MsgManager();

  bool AddSubInfo(const std::shared_ptr<Msg>& msg);

  bool AddPubInfo(const std::shared_ptr<Msg>& msg);

  void DispatchPubMsg(
    std::shared_ptr<Msg>,
    pub_cb_t pub_cb);

 private:
  std::string GetTopicName(const std::shared_ptr<Msg>& msg, bool sub_msg);
  // key: 发布topic
  // value: 订阅组件地址列表
  std::unordered_map<std::string, sub_info_set_t> sub_list_;
};

}  // namespace myframe
