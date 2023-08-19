/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <iostream>
#include <any>
#include <string>

#include "myframe/export.h"

namespace myframe {

/* 发送给框架的地址 */
const char* const MAIN_ADDR = "main";

/**
 * 发送给框架的命令
 * 发送示例:
 *  auto mailbox = GetMailbox();
 *  auto msg = std::make_shared<Msg>(MAIN_CMD_ALL_USER_MOD_ADDR);
 *  mailbox->Send(MAIN_ADDR, msg);
 * MAIN_CMD_ALL_USER_MOD_ADDR:
 *  返回用户所有模块列表
 *  通过msg->GetData()获得列表
 *  格式为 地址1\n地址2\n地址3
 */
const char* const MAIN_CMD_ALL_USER_MOD_ADDR = "kAllUserModAddr";

class MYFRAME_EXPORT Msg final {
 public:
  Msg() = default;
  Msg(const char* data);
  Msg(const char* data, int len);
  Msg(const std::string& data);

  /**
   * @brief 获得消息源地址
   * @note 来源：actor/worker/timer
   * @return const std::string& 源地址
   */
  const std::string& GetSrc() const { return src_; }
  const std::string& GetDst() const { return dst_; }

  /**
   * @brief 消息类型
   * @note 目前使用到的 "TEXT", "TIMER";
   * 也可以自定义，用于区分传递给同一个actor的不同消息类型
   * @return const std::string& 消息类型
   */
  const std::string& GetType() const { return type_; }

  /**
   * @brief 消息描述
   * @note 目前timer使用到该函数，见 Actor::Timeout()
   *
   * @return const std::string& 消息描述
   */
  const std::string& GetDesc() const { return desc_; }

  /**
   * @brief 数据
   *
   * @return const std::string& 数据
   */
  const std::string& GetData() const { return data_; }
  template <typename T>
  const T GetAnyData() const {
    return std::any_cast<T>(any_data_);
  }

  void SetSrc(const std::string& src) { src_ = src; }
  void SetDst(const std::string& dst) { dst_ = dst; }
  void SetType(const std::string& type) { type_ = type; }
  void SetDesc(const std::string& desc) { desc_ = desc; }
  void SetData(const char* data, unsigned int len);
  void SetData(const std::string& data);
  void SetAnyData(const std::any& any_data);

 private:
  std::string src_;
  std::string dst_;
  std::string type_;
  std::string desc_;
  std::string data_;
  std::any any_data_;
};

MYFRAME_EXPORT std::ostream& operator<<(std::ostream& out, const Msg& msg);

}  // namespace myframe
