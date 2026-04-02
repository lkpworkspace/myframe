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

class MYFRAME_EXPORT Msg final {
 public:
  enum class TransMode : int {
    kHybrid = 0,  // 混合模式
    kIntra = 1,  // 进程内传递
    kDDS = 2,  // 进程间/机器间传递
  };

  Msg() = default;
  Msg(const char* data, int len);
  Msg(const std::string& data);
  Msg(Msg&& o);
  Msg(const Msg& o);

  /**
   * @brief 获得消息分发模式
   * @note 来源：actor/worker/timer
   * @return 消息分发模式
   */
  TransMode GetTransMode() const { return trans_mode_; }

  /**
   * @brief 获得消息源地址
   * @note 来源：actor/worker/timer
   * @return const std::string& 源地址
   */
  const std::string& GetSrc() const { return src_; }

  /**
   * @brief 获得消息目的地址
   * @note 来源：actor/worker/timer
   * @return const std::string& 目的地址
   */
  const std::string& GetDst() const { return dst_; }

  /**
   * @brief 消息名称
   * @note 目前Actor的Subscribe, Publish和Timeout有使用到，
   * 见 Actor::Subscribe() / Actor::Publish() / Actor::Timeout()，
   * 也可以自定义，用于区分传递给同一个actor的不同消息
   * @return const std::string& 消息名称
   */
  const std::string& GetName() const { return name_; }

  /**
   * @brief 消息类型
   * @note 参考 MYFRAME_MSG_TYPE_* 宏定义;
   * 也可以自定义，用于区分传递给同一个actor的不同消息类型
   * @return const std::string& 消息类型
   */
  const std::string& GetType() const { return type_; }

  /**
   * @brief 消息描述
   * @note 自定义
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

  void SetTransMode(TransMode tans_mode) { trans_mode_ = tans_mode; }
  void SetSrc(const std::string& src) { src_ = src; }
  void SetDst(const std::string& dst) { dst_ = dst; }
  void SetName(const std::string& name) { name_ = name; }
  void SetType(const std::string& type) { type_ = type; }
  void SetDesc(const std::string& desc) { desc_ = desc; }
  void SetData(const char* data, unsigned int len);
  void SetData(const std::string& data);
  void SetAnyData(const std::any& any_data);

  Msg& operator=(Msg&& o) noexcept;
  Msg& operator=(const Msg& o) noexcept;

 private:
  std::string src_;
  std::string dst_;
  std::string name_;
  std::string type_;
  std::string desc_;
  std::string data_;
  std::any any_data_;
  TransMode trans_mode_{TransMode::kIntra};
};

MYFRAME_EXPORT std::ostream& operator<<(std::ostream& out, const Msg& msg);

}  // namespace myframe
