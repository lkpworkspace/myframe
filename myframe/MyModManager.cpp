#include "MyModManager.h"
#include <jsoncpp/json/json.h>
#include "MyLog.h"
#include "MyActor.h"
#include "MyWorker.h"

namespace myframe {

MyModManager::MyModManager() {
    pthread_rwlock_init(&_class_actor_rw, NULL);
    pthread_rwlock_init(&_class_worker_rw, NULL);
}

MyModManager::~MyModManager() {
    pthread_rwlock_destroy(&_class_actor_rw);
    pthread_rwlock_destroy(&_class_worker_rw);
}

bool MyModManager::LoadMod(const std::string& dl_path) {
    return _lib_mods.LoadMod(dl_path);
}

bool MyModManager::RegActor(const std::string& class_name, std::function<std::shared_ptr<MyActor>(const std::string&)> func) {
    pthread_rwlock_wrlock(&_class_actor_rw);
    if (_class_actors.find(class_name) != _class_actors.end()) {
        pthread_rwlock_unlock(&_class_actor_rw);
        LOG(WARNING) << "reg " << class_name << " failed, " << " has exist";
        return false;
    }
    _class_actors[class_name] = func;
    pthread_rwlock_unlock(&_class_actor_rw);
    return true;
}

bool MyModManager::RegWorker(const std::string& class_name, std::function<std::shared_ptr<MyWorker>(const std::string&)> func) {
    pthread_rwlock_wrlock(&_class_worker_rw);
    if (_class_workers.find(class_name) != _class_workers.end()) {
        pthread_rwlock_unlock(&_class_worker_rw);
        LOG(WARNING) << "reg " << class_name << " failed, " << " has exist";
        return false;
    }
    _class_workers[class_name] = func;
    pthread_rwlock_unlock(&_class_worker_rw);
    return true;
}

std::shared_ptr<MyActor> MyModManager::CreateActorInst(const std::string& mod_or_class_name, const std::string& actor_name) {
    if (_lib_mods.IsLoad(mod_or_class_name)) {
        LOG(INFO) << "instance actor from lib";
        return _lib_mods.CreateActorInst(mod_or_class_name, actor_name);
    }
    pthread_rwlock_rdlock(&_class_actor_rw);
    if (mod_or_class_name == "class" && _class_actors.find(actor_name) != _class_actors.end()) {
        LOG(INFO) << "instance actor from reg class";
        auto actor = _class_actors[actor_name](actor_name);
        actor->SetModName(mod_or_class_name);
        actor->SetTypeName(actor_name);
        pthread_rwlock_unlock(&_class_actor_rw);
        return actor;
    }
    pthread_rwlock_unlock(&_class_actor_rw);
    return nullptr;
}

std::shared_ptr<MyWorker> MyModManager::CreateWorkerInst(const std::string& mod_or_class_name, const std::string& worker_name) {
    if (_lib_mods.IsLoad(mod_or_class_name)) {
        LOG(INFO) << "instance worker from lib";
        return _lib_mods.CreateWorkerInst(mod_or_class_name, worker_name);
    }
    pthread_rwlock_rdlock(&_class_worker_rw);
    if (mod_or_class_name == "class" && _class_workers.find(worker_name) != _class_workers.end()) {
        LOG(INFO) << "instance worker from reg class";
        auto worker = _class_workers[worker_name](worker_name);
        worker->SetModName(mod_or_class_name);
        worker->SetTypeName(worker_name);
        pthread_rwlock_unlock(&_class_worker_rw);
        return worker;
    }
    pthread_rwlock_unlock(&_class_worker_rw);
    return nullptr;
}

} // namespace myframe