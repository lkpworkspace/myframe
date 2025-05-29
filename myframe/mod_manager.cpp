/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/mod_manager.h"

#include "myframe/common.h"
#include "myframe/log.h"
#include "myframe/shared_library.h"
#include "myframe/actor.h"
#include "myframe/actor_context.h"
#include "myframe/worker.h"
#include "myframe/worker_context.h"

namespace myframe {

ModManager::ModManager() {
  LOG(INFO) << "ModManager create";
}

ModManager::~ModManager() {
  LOG(INFO) << "ModManager deconstruct";
}

bool ModManager::LoadMod(const std::string& dl_path) {
  auto dlname = stdfs::path(dl_path).filename().string();
  std::unique_lock<std::shared_mutex> lk(mods_rw_);
  if (mods_.find(dlname) != mods_.end()) {
    VLOG(1) << dlname << " has loaded";
    return true;
  }
  auto lib = SharedLibrary::Create();
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
  const std::string& mod_or_class_name, const std::string& class_name) {
  {
    std::shared_lock<std::shared_mutex> lk(mods_rw_);
    if (mods_.find(mod_or_class_name) != mods_.end()) {
      VLOG(1) << class_name << " actor from lib";
      auto lib = mods_[mod_or_class_name];
      auto void_func = lib->GetSymbol("actor_create");
      auto create = reinterpret_cast<actor_create_func_t>(void_func);
      if (nullptr == create) {
        LOG(ERROR)
          << "Load " << mod_or_class_name << "." << class_name
          << " module actor_create function failed";
        return nullptr;
      }
      auto actor = create(class_name);
      if (nullptr == actor) {
        LOG(ERROR)
          << "Create " << mod_or_class_name << "." << class_name
          << " failed";
        return nullptr;
      }
      actor->SetModName(mod_or_class_name);
      actor->SetTypeName(class_name);
      return actor;
    }
  }
  std::shared_lock<std::shared_mutex> lk(class_actor_rw_);
  if (mod_or_class_name == "class" &&
      class_actors_.find(class_name) != class_actors_.end()) {
    VLOG(1) << class_name << " actor from reg class";
    auto actor = class_actors_[class_name](class_name);
    actor->SetModName(mod_or_class_name);
    actor->SetTypeName(class_name);
    return actor;
  }
  return nullptr;
}

std::shared_ptr<Worker> ModManager::CreateWorkerInst(
  const std::string& mod_or_class_name, const std::string& class_name) {
  {
    std::shared_lock<std::shared_mutex> lk(mods_rw_);
    if (mods_.find(mod_or_class_name) != mods_.end()) {
      VLOG(1) << class_name << " worker from lib";
      auto lib = mods_[mod_or_class_name];
      auto void_func = lib->GetSymbol("worker_create");
      auto create = reinterpret_cast<worker_create_func_t>(void_func);
      if (nullptr == create) {
        LOG(ERROR)
          << "Load " << mod_or_class_name << "." << class_name
          << " module worker_create function failed";
        return nullptr;
      }
      auto worker = create(class_name);
      if (nullptr == worker) {
        LOG(ERROR)
          << "Create " << mod_or_class_name << "." << class_name
          << " failed";
        return nullptr;
      }
      worker->SetModName(mod_or_class_name);
      worker->SetTypeName(class_name);
      return worker;
    }
  }
  std::shared_lock<std::shared_mutex> lk(class_worker_rw_);
  if (mod_or_class_name == "class" &&
      class_workers_.find(class_name) != class_workers_.end()) {
    VLOG(1) << class_name << " worker from reg class";
    auto worker = class_workers_[class_name](class_name);
    worker->SetModName(mod_or_class_name);
    worker->SetTypeName(class_name);
    return worker;
  }
  return nullptr;
}

bool ModManager::LoadService(
    const stdfs::path& lib_dir,
    const Json::Value& service,
    std::vector<std::pair<Json::Value, std::shared_ptr<Actor>>>* actors,
    std::vector<std::pair<Json::Value, std::shared_ptr<Worker>>>* workers) {
  if (service.isNull()) {
    LOG(ERROR) << "parse service json failed, skip";
    return false;
  }
  if (!service.isMember("type") || !service["type"].isString()) {
    LOG(ERROR) << "key \"type\": no key or not string, skip";
    return false;
  }
  const auto& type = service["type"].asString();
  std::string lib_name;
  // load library
  if (type == "library") {
    if (!service.isMember("lib") || !service["lib"].isString()) {
      LOG(ERROR) << " key \"lib\": no key or not string, skip";
      return false;
    }
    lib_name = service["lib"].asString();
    lib_name = Common::GetLibName(lib_name);
    if (!LoadMod((lib_dir / lib_name).string())) {
      LOG(ERROR) << "load lib "
        << (lib_dir / lib_name).string() << " failed, skip";
      return false;
    }
  }
  bool res = true;
  // load actor
  if (service.isMember("actor") && service["actor"].isObject()) {
    const auto& actor_list = service["actor"];
    Json::Value::Members actor_name_list = actor_list.getMemberNames();
    for (auto class_name_it = actor_name_list.begin();
          class_name_it != actor_name_list.end(); ++class_name_it) {
      if (type == "library") {
        res = LoadActors(
          lib_name,
          *class_name_it,
          actor_list,
          actors);
      } else if (type == "class") {
        res = LoadActors(
          "class",
          *class_name_it,
          actor_list,
          actors);
      } else {
        LOG(ERROR) << "Unknown type " << type;
        res = false;
      }
      if (!res) {
        return res;
      }
    }
  }
  // load worker
  if (service.isMember("worker") && service["worker"].isObject()) {
    const auto& worker_list = service["worker"];
    Json::Value::Members worker_name_list = worker_list.getMemberNames();
    for (auto class_name_it = worker_name_list.begin();
          class_name_it != worker_name_list.end(); ++class_name_it) {
      if (type == "library") {
        res = LoadWorkers(
          lib_name,
          *class_name_it,
          worker_list,
          workers);
      } else if (type == "class") {
        res = LoadWorkers(
          "class",
          *class_name_it,
          worker_list,
          workers);
      } else {
        LOG(ERROR) << "Unknown type " << type;
        res = false;
      }
      if (!res) {
        return res;
      }
    }  // end for
  }  // end load worker
  return res;
}

bool ModManager::LoadActors(
  const std::string& mod_name,
  const std::string& class_name,
  const Json::Value& actor_list,
  std::vector<std::pair<Json::Value, std::shared_ptr<Actor>>>* actors) {
  const auto& insts = actor_list[class_name];
  for (const auto& inst : insts) {
    std::string inst_name;
    std::string inst_param;
    Json::Value cfg;
    LOG(INFO)
      << "create actor instance \"" << class_name
      << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR)
        << "actor " << class_name
        << " key \"instance_name\": no key, skip";
      return false;
    }
    inst_name = inst["instance_name"].asString();
    if (inst.isMember("instance_params")) {
      inst_param = inst["instance_params"].asString();
    }
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    auto actor_inst = CreateActorInst(mod_name, class_name);
    if (actor_inst == nullptr) {
      LOG(ERROR) << "Create actor " << mod_name << "." << class_name << " failed";
      return false;
    }
    actor_inst->SetInstName(inst_name);
    actors->emplace_back(inst, actor_inst);
  }
  return true;
}

bool ModManager::LoadWorkers(
  const std::string& mod_name,
  const std::string& class_name,
  const Json::Value& worker_list,
  std::vector<std::pair<Json::Value, std::shared_ptr<Worker>>>* workers) {
  const auto& insts = worker_list[class_name];
  for (const auto& inst : insts) {
    std::string inst_name;
    Json::Value cfg;
    LOG(INFO)
      << "create worker instance \"" << class_name
      << "\": " << inst.toStyledString();
    if (!inst.isMember("instance_name")) {
      LOG(ERROR)
        << "worker " << class_name
        << " key \"instance_name\": no key, skip";
      return false;
    }
    inst_name = inst["instance_name"].asString();
    if (inst.isMember("instance_config")) {
      cfg = inst["instance_config"];
    }
    auto worker = CreateWorkerInst(mod_name, class_name);
    if (worker == nullptr) {
      LOG(ERROR)
        << "Create worker " << mod_name << "." << class_name << "."
        << inst_name << " failed";
      return false;
    }
    worker->SetInstName(inst_name);
    workers->emplace_back(inst, worker);
  }
  return true;
}

}  // namespace myframe
