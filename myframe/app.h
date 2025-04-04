/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <list>
#include <string>
#include <vector>

#include <json/json.h>

#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/export.h"
#include "myframe/common.h"

namespace myframe {

class Msg;
class Poller;
class Actor;
class ActorContext;
class ActorContextManager;
class Event;
class EventManager;
class EventConn;
class EventConnManager;
class Worker;
class WorkerContext;
class WorkerCommon;
class WorkerTimer;
class WorkerContextManager;
class ModManager;
class MYFRAME_EXPORT App final : public std::enable_shared_from_this<App> {
  friend class Actor;

 public:
  enum class State : std::uint8_t {
    kUninitialized = 0,
    kInitialized,
    kRunning,
    kQuitting,
    kQuit,
  };

  App();
  virtual ~App();

  bool Init(
    const std::string& lib_dir,
    int thread_pool_size = 4,
    int event_conn_size = 2,
    int warning_msg_size = 10,
    int default_pending_queue_size = -1,
    int default_run_queue_size = 2);

  int LoadServiceFromDir(const std::string& path);

  bool LoadServiceFromFile(const std::string& file);

  bool LoadServiceFromJsonStr(const std::string& service);

  bool LoadServiceFromJson(const Json::Value& service);

  bool AddActor(
    const std::string& inst_name,
    const std::string& params,
    std::shared_ptr<Actor> actor,
    const Json::Value& config = Json::Value::nullSingleton());

  bool AddWorker(
    const std::string& inst_name,
    std::shared_ptr<Worker> worker,
    const Json::Value& config = Json::Value::nullSingleton());

  int Send(std::shared_ptr<Msg> msg);

  const std::shared_ptr<const Msg> SendRequest(
    std::shared_ptr<Msg> msg);

  std::unique_ptr<ModManager>& GetModManager();

  int Exec();

  void Quit();

  int GetDefaultPendingQueueSize() const;
  int GetDefaultRunQueueSize() const;

 private:
  bool HasUserInst(const std::string& name);
  std::shared_ptr<WorkerTimer> GetTimerWorker();

  bool LoadActors(
    const std::string& mod_name,
    const std::string& actor_name,
    const Json::Value& actor_list);
  bool LoadWorkers(
    const std::string& mod_name,
    const std::string& worker_name,
    const Json::Value& worker_list);
  std::string GetLibName(const std::string& name);

  /// worker
  bool StartCommonWorker(int worker_count);
  bool StartTimerWorker();

  /// 通知执行事件
  void CheckStopWorkers();

  /// 分发消息
  void DispatchMsg(std::shared_ptr<Msg> msg);
  void DispatchMsg(std::list<std::shared_ptr<Msg>>* msg_list);
  void DispatchMsg(std::shared_ptr<ActorContext> context);
  /// 处理事件
  void ProcessEvent(const std::vector<ev_handle_t>& evs);
  void ProcessWorkerEvent(std::shared_ptr<WorkerContext>);
  void ProcessTimerEvent(std::shared_ptr<WorkerContext>);
  void ProcessUserEvent(std::shared_ptr<WorkerContext>);
  void ProcessEventConn(std::shared_ptr<EventConn>);
  void ProcessMain(std::shared_ptr<Msg>);
  void GetAllUserModAddr(std::string* info);

  stdfs::path lib_dir_;
  /// node地址
  std::string node_addr_;
  ///
  int default_pending_queue_size_{-1};
  int default_run_queue_size_{2};
  std::atomic<std::size_t> warning_msg_size_{10};
  std::atomic<State> state_{State::kUninitialized};
  std::recursive_mutex local_mtx_;
  /// 缓存消息列表
  std::list<std::shared_ptr<Msg>> cache_msgs_;
  /// 模块管理对象
  std::unique_ptr<ModManager> mods_;
  /// poller
  std::shared_ptr<Poller> poller_;
  /// 句柄管理对象
  std::unique_ptr<ActorContextManager> actor_ctx_mgr_;
  /// 事件管理对象
  std::shared_ptr<EventManager> ev_mgr_;
  /// 与框架通信管理对象
  std::unique_ptr<EventConnManager> ev_conn_mgr_;
  /// 线程管理对象
  std::unique_ptr<WorkerContextManager> worker_ctx_mgr_;

  DISALLOW_COPY_AND_ASSIGN(App)
};

}  // namespace myframe
