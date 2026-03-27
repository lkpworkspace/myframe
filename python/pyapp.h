/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include "myframe/common.h"
#include "myframe/log.h"
#include "myframe/app.h"
#include "myframe/mod_manager.h"
#include "pymsg.h"
#include "pyactor.h"

namespace pymyframe {

class App;
class AppConf {
  friend class App;
 public:
  AppConf() {
    log_dir_ = myframe::Common::GetAbsolutePath(log_dir_).string();
    // std::cout << "appconf construct\n";
  }
  ~AppConf() {
    // std::cout << "appconf deconstruct\n";
  }

  void setProcessName(const std::string& name) {
    process_name_ = name;
  }

  void setLibDir(const std::string& dir) {
    args_.SetStr(MYFRAME_KEY_SERVICE_LIB_DIR,
      myframe::Common::GetAbsolutePath(dir).string());
  }

  void setLogDir(const std::string& dir) {
    log_dir_ = myframe::Common::GetAbsolutePath(dir).string();
  }

  void setLogMaxSizeMB(int sz) {
    log_max_size_ = sz;
  }

  void setThreadPoolSize(int sz) {
    args_.SetInt(MYFRAME_KEY_THREAD_POOL_SIZE, sz);
  }

  void setPendingQueueSize(int sz) {
    args_.SetInt(MYFRAME_KEY_PENDING_QUEUE_SIZE, sz);
  }

  void setRunQueueSize(int sz) {
    args_.SetInt(MYFRAME_KEY_RUN_QUEUE_SIZE, sz);
  }

 private:
  int log_max_size_{100};
  std::string log_dir_{"log"};
  std::string process_name_{"launcher"};
  myframe::Arguments args_;
};

class App {
 public:
  App() {
    // std::cout << "pyapp construct\n";
  }

  ~App() {
    if (app_) {
      app_->Quit();
    }
    if (th_.joinable()) {
      th_.join();
    }
    // std::cout << "pyapp deconstruct\n";
  }

  bool init(const AppConf& conf) {
    // 初始化Log
    myframe::InitLog(
      conf.log_dir_,
      conf.process_name_,
      conf.log_max_size_);
    // 初始化App
    app_ = std::make_shared<myframe::App>();
    bool res = app_->Init(conf.args_);
    if (res == false) {
      return false;
    }
    // 注册PyActor
    auto& mod = app_->GetModManager();
    res = mod->RegActor("PyActor", [](const std::string&) {
      return std::make_shared<PyActor>();
    });
    return res;
  }

  bool loadServiceFromDir(const std::string& path) {
    if (app_ == nullptr) {
      return false;
    }
    return app_->LoadServiceFromDir(
      myframe::Common::GetAbsolutePath(path).string());
  }

  bool loadServiceFromFile(const std::string& filepath) {
    if (app_ == nullptr) {
      return false;
    }
    return app_->LoadServiceFromFile(
      myframe::Common::GetAbsolutePath(filepath).string());
  }

  int send(const pymyframe::Msg& msg) {
    if (app_ == nullptr) {
      return -1;
    }
    return app_->Send(msg.msg_);
  }

  // TODO(likepeng)
  // msg sendRequest(msg);

  bool addActor(PyObject* py_actor_obj, const std::string& py_actor_conf) {
    if (app_ == nullptr) {
      return false;
    }
    if (!py_actor_obj) {
      return false;
    }
    // 解析配置
    auto json_obj = myframe::Common::LoadJsonFromString(py_actor_conf);
    if (json_obj.isNull()) {
      return false;
    }
    std::string instance_name;
    if (json_obj.isMember("instance_name")
        && json_obj["instance_name"].isString()) {
      instance_name = json_obj["instance_name"].asString();
    }
    // 创建PyActor对象并添加到框架
    auto& mod = app_->GetModManager();
    auto actor = mod->CreateActorInst(
      "class", "PyActor", instance_name);
    if (!actor) {
      return false;
    }
    auto pyactor = std::dynamic_pointer_cast<pymyframe::PyActor>(actor);
    pyactor->SetPyObj(py_actor_obj);
    return app_->AddActor(actor, json_obj);
  }

  void start() {
    if (app_ == nullptr) {
      return;
    }
    th_ = std::thread([this](){
      app_->Exec();
    });
  }

 private:
  std::shared_ptr<myframe::App> app_;
  std::thread th_;
};

}  // namespace pymyframe
