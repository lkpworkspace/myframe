/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <string>

#include <jsoncpp/json/json.h>

#include "myframe/macros.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"
#include "myframe/event.h"

namespace myframe {

class WorkerContext;
class Worker {
  friend class App;
  friend class ModLib;
  friend class ModManager;
  friend class WorkerContext;

 public:
  Worker() = default;
  virtual ~Worker();

  /**
   * GetType() - 获得事件类型
   *
   * @return:    事件类型
   */
  virtual EventType GetType();

  /**
  * GetConfig() - 获得配置参数
  * @return: 返回json对象
  */
  const Json::Value* GetConfig() const;

  /**
   * GetWorkerName() - 获得该worker的worker名
   *
   * @return:         成功返回：worker名，失败返回：空字符串
   */
  const std::string GetWorkerName() const;
  const std::string& GetModName() const;
  const std::string& GetTypeName() const;
  const std::string& GetInstName() const;

 protected:
  /**
   * Mailbox() - 发送消息的mailbox
   *
   * @return:         失败 nullptr
   */
  Mailbox* GetMailbox();

  /**
   * GetCmdChannel() - 与框架通信对象
   *
   * @return:         失败 nullptr
   */
  CmdChannel* GetCmdChannel();

  /**
   * Init() - worker初始化
   * 
   *  在新创建的线程中调用该初始化函数
   */
  virtual void Init() {}

  /**
   * Run() - worker线程循环调用
   * 
   *  函数会循环调用,调用Stop停止调用
   */
  virtual void Run() = 0;

  /**
   * Exit() - worker线程退出时调用
   */
  virtual void Exit() {}

  /**
   * Stop() - 停止Run循环调用
   */
  void Stop();

  /// 分发消息并立即返回
  int DispatchMsg();

  /// 分发消息并等待回复消息
  int DispatchAndWaitMsg();

 private:
  void SetModName(const std::string&);
  void SetTypeName(const std::string&);
  void SetInstName(const std::string&);
  void SetConfig(const Json::Value&);

  void SetContext(std::shared_ptr<WorkerContext>);

  std::string mod_name_;
  std::string worker_name_;
  std::string inst_name_;
  Json::Value config_{ Json::Value::null };

  std::weak_ptr<WorkerContext> ctx_;

  DISALLOW_COPY_AND_ASSIGN(Worker)
};

}  // namespace myframe

extern "C" {
typedef std::shared_ptr<myframe::Worker> (*my_worker_create_func)(
    const std::string&);
}  // extern "C"
