/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <iostream>
#include <any>
#include <string>

namespace myframe {

class Msg final {
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
   * @note 目前myframe timer使用到该函数，见 Actor::Timeout()
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

std::ostream& operator<<(std::ostream& out, const Msg& msg);

}  // namespace myframe
