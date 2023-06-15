/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/mod_manager.h"

#include <glog/logging.h>
#include <jsoncpp/json/json.h>

#include "myframe/actor.h"
#include "myframe/worker.h"

namespace myframe {

ModManager::ModManager() {
  LOG(INFO) << "ModManager create";
}

ModManager::~ModManager() {
  LOG(INFO) << "ModManager deconstruct";
}

bool ModManager::LoadMod(const std::string& dl_path) {
  return lib_mods_.LoadMod(dl_path);
}

bool ModManager::RegActor(
  const std::string& class_name,
  std::function<std::shared_ptr<Actor>(const std::string&)> func) {
  std::unique_lock<std::shared_mutex> lk(class_actor_rw_);
  if (class_actors_.find(class_name) != class_actors_.end()) {
    LOG(WARNING)
      << "reg " << class_name << " failed, "
      << " has exist";
    return false;
  }
  class_actors_[class_name] = func;
  return true;
}

bool ModManager::RegWorker(
  const std::string& class_name,
  std::function<std::shared_ptr<Worker>(const std::string&)> func) {
  std::unique_lock<std::shared_mutex> lk(class_worker_rw_);
  if (class_workers_.find(class_name) != class_workers_.end()) {
    LOG(WARNING)
      << "reg " << class_name << " failed, "
      << " has exist";
    return false;
  }
  class_workers_[class_name] = func;
  return true;
}

std::shared_ptr<Actor> ModManager::CreateActorInst(
  const std::string& mod_or_class_name, const std::string& actor_name) {
  if (lib_mods_.IsLoad(mod_or_class_name)) {
    DLOG(INFO) << actor_name << " actor from lib";
    return lib_mods_.CreateActorInst(mod_or_class_name, actor_name);
  }
  std::shared_lock<std::shared_mutex> lk(class_actor_rw_);
  if (mod_or_class_name == "class" &&
      class_actors_.find(actor_name) != class_actors_.end()) {
    DLOG(INFO) << actor_name << " actor from reg class";
    auto actor = class_actors_[actor_name](actor_name);
    actor->SetModName(mod_or_class_name);
    actor->SetTypeName(actor_name);
    return actor;
  }
  return nullptr;
}

std::shared_ptr<Worker> ModManager::CreateWorkerInst(
  const std::string& mod_or_class_name, const std::string& worker_name) {
  if (lib_mods_.IsLoad(mod_or_class_name)) {
    LOG(INFO) << "instance worker from lib";
    return lib_mods_.CreateWorkerInst(mod_or_class_name, worker_name);
  }
  std::shared_lock<std::shared_mutex> lk(class_worker_rw_);
  if (mod_or_class_name == "class" &&
      class_workers_.find(worker_name) != class_workers_.end()) {
    LOG(INFO) << "instance worker from reg class";
    auto worker = class_workers_[worker_name](worker_name);
    worker->SetModName(mod_or_class_name);
    worker->SetTypeName(worker_name);
    return worker;
  }
  return nullptr;
}

}  // namespace myframe
