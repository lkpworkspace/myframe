/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/mod_manager.h"

#include <glog/logging.h>
#include <jsoncpp/json/json.h>

#include "myframe/shared_library.h"
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
  auto dlname = GetLibName(dl_path);
  std::unique_lock<std::shared_mutex> lk(mods_rw_);
  if (mods_.find(dlname) != mods_.end()) {
    DLOG(INFO) << dlname << " has loaded";
    return true;
  }
  auto lib = std::make_shared<SharedLibrary>();
  if (!lib->Load(dl_path, SharedLibrary::Flags::kLocal)) {
    return false;
  }
  mods_[dlname] = lib;
  LOG(INFO) << "Load lib " << dl_path;
  return true;
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
  {
    std::shared_lock<std::shared_mutex> lk(mods_rw_);
    if (mods_.find(mod_or_class_name) != mods_.end()) {
      DLOG(INFO) << actor_name << " actor from lib";
      auto lib = mods_[mod_or_class_name];
      auto void_func = lib->GetSymbol("actor_create");
      auto create = reinterpret_cast<actor_create_func_t>(void_func);
      if (nullptr == create) {
        LOG(ERROR)
          << "Load " << mod_or_class_name << "." << actor_name
          << " module actor_create function failed";
        return nullptr;
      }
      auto actor = create(actor_name);
      if (nullptr == actor) {
        LOG(ERROR)
          << "Create " << mod_or_class_name << "." << actor_name
          << " failed";
        return nullptr;
      }
      actor->SetModName(mod_or_class_name);
      actor->SetTypeName(actor_name);
      return actor;
    }
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
  {
    std::shared_lock<std::shared_mutex> lk(mods_rw_);
    if (mods_.find(mod_or_class_name) != mods_.end()) {
      LOG(INFO) << "instance worker from lib";
      auto lib = mods_[mod_or_class_name];
      auto void_func = lib->GetSymbol("worker_create");
      auto create = reinterpret_cast<worker_create_func_t>(void_func);
      if (nullptr == create) {
        LOG(ERROR)
          << "Load " << mod_or_class_name << "." << worker_name
          << " module worker_create function failed";
        return nullptr;
      }
      auto worker = create(worker_name);
      if (nullptr == worker) {
        LOG(ERROR)
          << "Create " << mod_or_class_name << "." << worker_name
          << " failed";
        return nullptr;
      }
      worker->SetModName(mod_or_class_name);
      worker->SetTypeName(worker_name);
      return worker;
    }
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

std::string ModManager::GetLibName(const std::string& path) const {
  auto pos = path.find_last_of('/');
  pos = (pos == std::string::npos) ? -1 : pos;
  return path.substr(pos + 1);
}

}  // namespace myframe
