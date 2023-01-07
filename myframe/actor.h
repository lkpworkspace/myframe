/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <stddef.h>
#include <stdint.h>

#include <any>
#include <memory>
#include <string>

namespace myframe {

class Msg;
class Context;
class Actor {
  friend class App;
  friend class Context;
  friend class ModLib;
  friend class ModManager;

 public:
  Actor();
  virtual ~Actor();

  /**
   * Init() - actor初始化调用的初始化函数
   * @c:      actor指针
   * @param:  actor参数
   *
   * @return: 未定义
   */
  virtual int Init(const char* param) = 0;

  /**
   * Proc() - 消息处理函数
   * @msg:      actor收到的消息
   *
   */
  virtual void Proc(const std::shared_ptr<const Msg>& msg) = 0;

  /**
   * Send() - 发送消息给别的actor
   * @dst:            目的actor, eg: actor.example.hellow_world
   * @msg:            发送的消息
   *
   *      将消息添加到该actor的消息发送队列中，等待actor执行完成后，myframe会将消息分发给其他actor
   *
   * @return:         成功 0， 失败 -1
   */
  int Send(const std::string& dst, std::shared_ptr<Msg> msg);
  int Send(const std::string& dst, std::any data);

  /**
   * GetActorName() - 获得该actor的actor名
   *
   * @return:         成功返回：actor名，失败返回：空字符串
   */
  const std::string GetActorName() const;
  const std::string& GetTypeName() const { return actor_name_; }
  const std::string& GetInstName() const { return instance_name_; }

  /**
   * Timeout() - 设置定时器
   * @expired: 超时时间(单位:10ms, 比如 expired = 1, 那么超时时间就是10ms)
   *
   *      定时器设置之后，过了超时时间，actor就会收到超时消息;
   *      如果想实现周期性的定时器，可以在收到超时消息之后，
   *      再次调用此函数设置下一次的超时。
   *
   *      msg->GetType() == "TIMER" 确认是定时器消息
   *      msg->GetDesc() == timer_name 确认是那个定时器消息
   *
   * @return:         成功返回: 0, 失败返回: -1
   */
  int Timeout(const std::string& timer_name, int expired);

 private:
  void SetModName(const std::string& name);
  void SetTypeName(const std::string& name) { actor_name_ = name; }
  void SetInstName(const std::string& name) { instance_name_ = name; }
  bool IsFromLib() { return is_from_lib_; }
  void SetContext(std::shared_ptr<Context>);

  bool is_from_lib_ = false;
  std::string mod_name_;
  std::string actor_name_;
  std::string instance_name_;
  std::weak_ptr<Context> ctx_;
};

}  // namespace myframe

extern "C" {
typedef std::shared_ptr<myframe::Actor> (*my_actor_create_func)(
    const std::string&);
}  // extern "C"
