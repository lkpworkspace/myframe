/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/mod_lib.h"

#include <dlfcn.h>

#include "myframe/actor.h"
#include "myframe/log.h"
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
  my_worker_create_func create =
      (my_worker_create_func)dlsym(handle, "my_worker_create");
  if (nullptr == create) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR) << "Load " << mod_name << "." << worker_name
               << " module my_worker_create function failed";
    return nullptr;
  }
  auto worker = create(worker_name);
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
  my_actor_create_func create =
      (my_actor_create_func)dlsym(handle, "my_actor_create");
  if (nullptr == create) {
    pthread_rwlock_unlock(&rw_);
    LOG(ERROR) << "Load " << mod_name << "." << actor_name
               << " module my_actor_create function failed";
    return nullptr;
  }
  auto mod = create(actor_name);
  mod->SetModName(mod_name);
  mod->SetTypeName(actor_name);
  pthread_rwlock_unlock(&rw_);
  return mod;
}

}  // namespace myframe
