/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <string>

#include <json/json.h>

#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"
#include "myframe/event.h"

namespace myframe {

class App;
class WorkerContext;
class MYFRAME_EXPORT Worker {
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
  virtual Event::Type GetType();

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
  void SetModName(const std::string&);
  void SetTypeName(const std::string&);
  void SetInstName(const std::string&);
  void SetConfig(const Json::Value&);

  void SetContext(std::shared_ptr<WorkerContext>);

  std::string mod_name_;
  std::string worker_name_;
  std::string inst_name_;
  Json::Value config_;

  std::weak_ptr<WorkerContext> ctx_;

  DISALLOW_COPY_AND_ASSIGN(Worker)
};

}  // namespace myframe

#include "myframe/platform.h"
#if defined(MYFRAME_PLATFORM_WINDOWS)
template class std::shared_ptr<myframe::Worker>;
#endif
extern "C" {
typedef std::shared_ptr<myframe::Worker> (*worker_create_func_t)(
    const std::string&);
}  // extern "C"
