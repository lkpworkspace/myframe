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

struct epoll_event;

namespace myframe {

class Context;
class Msg;
class Actor;
class Event;
class Worker;
class WorkerCommon;
class WorkerTimer;
class ModManager;
class ContextManager;
class WorkerManager;
class EventConn;
class EventConnManager;
class App final : public std::enable_shared_from_this<App> {
  friend class Actor;
  friend class Context;

 public:
  App();
  virtual ~App();

  bool Init();

  bool LoadModsFromConf(const std::string& path);

  bool AddActor(const std::string& inst_name, const std::string& params,
                std::shared_ptr<Actor> actor);
  bool AddWorker(const std::string& inst_name,
                 std::shared_ptr<Worker> worker);

  std::shared_ptr<Msg> SendRequest(const std::string& name,
                                     std::shared_ptr<Msg> msg);

  std::unique_ptr<ContextManager>& GetContextManager() {
    return context_mgr_;
  }
  std::unique_ptr<ModManager>& GetModManager() { return mods_; }

  bool AddEvent(std::shared_ptr<Event> ev);
  bool DelEvent(std::shared_ptr<Event> ev);

  int Exec();

 private:
  bool CreateContext(const std::string& mod_name, const std::string& actor_name,
                     const std::string& instance_name,
                     const std::string& params);
  bool CreateContext(std::shared_ptr<Actor> mod_inst,
                     const std::string& params);

  std::shared_ptr<WorkerTimer> GetTimerWorker();

  bool LoadActorFromLib(const Json::Value& root, const Json::Value& actor_list,
                        const std::string& actor_name);
  bool LoadActorFromClass(const Json::Value& root,
                          const Json::Value& actor_list,
                          const std::string& actor_name);
  bool LoadWorkerFromLib(const Json::Value& root,
                         const Json::Value& worker_list,
                         const std::string& worker_name);
  bool LoadWorkerFromClass(const Json::Value& root,
                           const Json::Value& worker_list,
                           const std::string& worker_name);

  /// worker
  bool StartCommonWorker(int worker_count);
  bool StartTimerWorker();

  /// 通知执行事件
  void CheckStopWorkers();

  /// 分发事件
  void DispatchMsg(std::list<std::shared_ptr<Msg>>* msg_list);
  void DispatchMsg(std::shared_ptr<Context> context);
  void ProcessEvent(struct epoll_event* evs, int ev_count);
  void ProcessWorkerEvent(std::shared_ptr<WorkerCommon>);
  void ProcessTimerEvent(std::shared_ptr<WorkerTimer>);
  void ProcessUserEvent(std::shared_ptr<Worker>);
  void ProcessEventConn(std::shared_ptr<EventConn>);

  std::atomic_bool quit_ = {true};
  std::mutex dispatch_mtx_;
  /// epoll文件描述符
  int epoll_fd_;
  /// 句柄管理对象
  std::unique_ptr<ContextManager> context_mgr_;
  /// 模块管理对象
  std::unique_ptr<ModManager> mods_;
  /// 线程管理对象
  std::unique_ptr<WorkerManager> worker_mgr_;
  /// 与框架通信管理对象
  std::unique_ptr<EventConnManager> ev_conn_mgr_;
};

}  // namespace myframe
