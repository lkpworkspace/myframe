/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <any>
#include <memory>
#include <string>

#include <jsoncpp/json/json.h>

#include "myframe/macros.h"
#include "myframe/mailbox.h"

namespace myframe {

class Msg;
class ActorContext;
class App;
class Actor {
  friend class App;
  friend class ActorContext;
  friend class ModLib;
  friend class ModManager;

 public:
  Actor() = default;
  virtual ~Actor();

  /**
   * GetConfig() - 获得配置参数
   * @return: 返回json对象
   */
  const Json::Value* GetConfig() const;

  /**
   * GetActorName() - 获得该actor的actor名
   *
   * @return:         成功返回：actor名，失败返回：空字符串
   */
  const std::string GetActorName() const;
  const std::string& GetModName() const;
  const std::string& GetTypeName() const;
  const std::string& GetInstName() const;

 protected:
  /**
   * Init() - actor初始化调用的初始化函数
   * @c:      actor指针
   * @param:  actor参数
   *
   * @return:         成功 0， 失败 -1
   */
  virtual int Init(const char* param) = 0;

  /**
   * Proc() - 消息处理函数
   * @msg:      actor收到的消息
   *
   */
  virtual void Proc(const std::shared_ptr<const Msg>& msg) = 0;

  /**
   * Mailbox() - 发送消息的mailbox
   *
   * @return:         失败 nullptr
   */
  Mailbox* GetMailbox();

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

  /**
   * Subscribe() - 订阅actor或者worker的消息
   * @name: 订阅actor或者worker的名称
   *
   *     被订阅的组件需要处理订阅消息，消息格式：
   *        msg->GetType() == "SUBSCRIBE" 确认是订阅消息
   *        msg->GetSrc() 确定是订阅组件名称
   * @return: 成功返回true,失败返回false
   */
  bool Subscribe(const std::string& name);

  /**
   * GetApp() - 获得应用实例
   *
   *    注意：不要将返回的对象存储为成员变量或者静态变量，
   *        否则会导致程序退出异常。
   *
   * @return: 成功返回: app对象指针, 失败返回: nullptr
   */
  std::shared_ptr<App> GetApp();

 private:
  bool IsFromLib() const;
  void SetModName(const std::string& name);
  void SetTypeName(const std::string& name);
  void SetInstName(const std::string& name);
  void SetConfig(const Json::Value& conf);

  void SetContext(std::shared_ptr<ActorContext>);

  bool is_from_lib_{ false };
  std::string mod_name_;
  std::string actor_name_;
  std::string instance_name_;
  Json::Value config_{ Json::Value::null };
  std::weak_ptr<ActorContext> ctx_;

  DISALLOW_COPY_AND_ASSIGN(Actor)
};

}  // namespace myframe

extern "C" {
typedef std::shared_ptr<myframe::Actor> (*actor_create_func_t)(
    const std::string&);
}  // extern "C"
