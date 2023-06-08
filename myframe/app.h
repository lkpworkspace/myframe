/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

#include <jsoncpp/json/json.h>

#include "myframe/macros.h"
#include "myframe/event.h"

struct epoll_event;

namespace myframe {

class Msg;
class Actor;
class ActorContext;
class ActorContextManager;
class Event;
class EventConn;
class EventConnManager;
class Worker;
class WorkerContext;
class WorkerCommon;
class WorkerTimer;
class WorkerContextManager;
class ModManager;
class App final : public std::enable_shared_from_this<App> {
  friend class Actor;
  friend class ActorContext;

 public:
  App();
  virtual ~App();

  bool Init(
    const std::string& lib_dir,
    int thread_pool_size = 4,
    int event_conn_size = 2,
    int warning_msg_size = 10);

  int LoadServiceFromDir(const std::string& path);

  bool LoadServiceFromFile(const std::string& file);

  bool LoadServiceFromJson(const Json::Value& service);

  bool LoadServiceFromJsonStr(const std::string& service);

  bool AddActor(
    const std::string& inst_name,
    const std::string& params,
    std::shared_ptr<Actor> actor,
    const Json::Value& config = Json::Value::null);

  bool AddWorker(
    const std::string& inst_name,
    std::shared_ptr<Worker> worker,
    const Json::Value& config = Json::Value::null);

  int Send(
    const std::string& dst,
    std::shared_ptr<Msg> msg);

  const std::shared_ptr<const Msg> SendRequest(
    const std::string& name,
    std::shared_ptr<Msg> msg);

  std::unique_ptr<ActorContextManager>& GetActorContextManager() {
    return actor_ctx_mgr_;
  }

  std::unique_ptr<ModManager>& GetModManager() { return mods_; }

  bool AddEvent(std::shared_ptr<Event> ev);
  bool DelEvent(std::shared_ptr<Event> ev);

  int Exec();

  void Quit();

 private:
  bool CreateActorContext(
    const std::string& mod_name,
    const std::string& actor_name,
    const std::string& inst_name,
    const std::string& params,
    const Json::Value& config = Json::Value::null);
  bool CreateActorContext(
    std::shared_ptr<Actor> inst,
    const std::string& params);

  std::shared_ptr<WorkerTimer> GetTimerWorker();

  bool LoadActors(
    const std::string& mod_name,
    const std::string& actor_name,
    const Json::Value& actor_list);
  bool LoadWorkers(
    const std::string& mod_name,
    const std::string& worker_name,
    const Json::Value& worker_list);

  /// worker
  bool StartCommonWorker(int worker_count);
  bool StartTimerWorker();

  /// 通知执行事件
  void CheckStopWorkers();

  /// 分发事件
  EventIOType ToEventIOType(int ev);
  int ToEpollType(const EventIOType& type);
  void DispatchMsg(std::list<std::shared_ptr<Msg>>* msg_list);
  void DispatchMsg(std::shared_ptr<ActorContext> context);
  void ProcessEvent(struct epoll_event* evs, int ev_count);
  void ProcessWorkerEvent(std::shared_ptr<WorkerContext>);
  void ProcessTimerEvent(std::shared_ptr<WorkerContext>);
  void ProcessUserEvent(std::shared_ptr<WorkerContext>);
  void ProcessEventConn(std::shared_ptr<EventConn>);
  void ProcessMain(std::shared_ptr<Msg>);
  void GetAllUserModAddr(std::string* info);

  std::string lib_dir_;
  /// node地址
  std::string node_addr_;
  std::atomic<std::size_t> warning_msg_size_{10};
  std::atomic_bool quit_{true};
  std::mutex dispatch_mtx_;
  std::mutex local_mtx_;
  /// epoll文件描述符
  int epoll_fd_;
  /// 模块管理对象
  std::unique_ptr<ModManager> mods_;
  /// 句柄管理对象
  std::unique_ptr<ActorContextManager> actor_ctx_mgr_;
  /// 与框架通信管理对象
  std::unique_ptr<EventConnManager> ev_conn_mgr_;
  /// 线程管理对象
  std::unique_ptr<WorkerContextManager> worker_ctx_mgr_;

  DISALLOW_COPY_AND_ASSIGN(App)
};

}  // namespace myframe
