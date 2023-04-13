/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/mod_lib.h"

#include <dlfcn.h>

#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/worker.h"

namespace myframe {

ModLib::ModLib() { pthread_rwlock_init(&rw_, NULL); }

ModLib::~ModLib() {
  pthread_rwlock_wrlock(&rw_);
  for (const auto& p : mods_) {
    dlclose(p.second);
  }
  mods_.clear();
  pthread_rwlock_unlock(&rw_);
  pthread_rwlock_destroy(&rw_);
}

std::string ModLib::GetModName(const std::string& full_path) {
  auto pos = full_path.find_last_of('/');
  pos = (pos == std::string::npos) ? -1 : pos;
  return full_path.substr(pos + 1);
}

bool ModLib::LoadMod(const std::string& dlpath) {
  auto dlname = GetModName(dlpath);
  pthread_rwlock_wrlock(&rw_);
  if (mods_.find(dlname) != mods_.end()) {
    pthread_rwlock_unlock(&rw_);
    DLOG(INFO) << dlname << " has loaded";
    return true;
  }

  void* dll_handle = dlopen(dlpath.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (dll_handle == nullptr) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR) << "Open dll " << dlpath << " failed, " << dlerror();
    return false;
  }
  mods_[dlname] = dll_handle;
  pthread_rwlock_unlock(&rw_);
  LOG(INFO) << "Load lib " << dlpath;
  return true;
}

bool ModLib::IsLoad(const std::string& dlname) {
  pthread_rwlock_rdlock(&rw_);
  auto res = mods_.find(dlname) != mods_.end();
  pthread_rwlock_unlock(&rw_);
  return res;
}

bool ModLib::UnloadMod(const std::string& dlname) {
  pthread_rwlock_wrlock(&rw_);
  if (mods_.find(dlname) == mods_.end()) {
    pthread_rwlock_unlock(&rw_);
    return true;
  }

  if (dlclose(mods_[dlname])) {
    LOG(ERROR) << "lib close failed, " << dlerror();
  }
  mods_.erase(dlname);
  pthread_rwlock_unlock(&rw_);
  return true;
}

std::shared_ptr<Worker> ModLib::CreateWorkerInst(
    const std::string& mod_name, const std::string& worker_name) {
  pthread_rwlock_rdlock(&rw_);
  if (mods_.find(mod_name) == mods_.end()) {
    LOG(ERROR) << "Find " << mod_name << "." << worker_name << " failed";
    return nullptr;
  }
  void* handle = mods_[mod_name];
  auto void_func = dlsym(handle, "worker_create");
  auto create = reinterpret_cast<worker_create_func_t>(void_func);
  if (nullptr == create) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR)
      << "Load " << mod_name << "." << worker_name
      << " module worker_create function failed";
    return nullptr;
  }
  auto worker = create(worker_name);
  if (nullptr == worker) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR)
      << "Create " << mod_name << "." << worker_name
      << " failed";
    return nullptr;
  }
  worker->SetModName(mod_name);
  worker->SetTypeName(worker_name);
  pthread_rwlock_unlock(&rw_);
  return worker;
}

std::shared_ptr<Actor> ModLib::CreateActorInst(
    const std::string& mod_name, const std::string& actor_name) {
  pthread_rwlock_rdlock(&rw_);
  if (mods_.find(mod_name) == mods_.end()) {
    LOG(ERROR) << "Find " << mod_name << "." << actor_name << " failed";
    return nullptr;
  }
  void* handle = mods_[mod_name];
  auto void_func = dlsym(handle, "actor_create");
  auto create = reinterpret_cast<actor_create_func_t>(void_func);
  if (nullptr == create) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR)
      << "Load " << mod_name << "." << actor_name
      << " module actor_create function failed";
    return nullptr;
  }
  auto actor = create(actor_name);
  if (nullptr == actor) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR)
      << "Create " << mod_name << "." << actor_name
      << " failed";
    return nullptr;
  }
  actor->SetModName(mod_name);
  actor->SetTypeName(actor_name);
  pthread_rwlock_unlock(&rw_);
  return actor;
}

}  // namespace myframe
