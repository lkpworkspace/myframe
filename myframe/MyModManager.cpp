#include "MyModManager.h"
#include <jsoncpp/json/json.h>
#include "MyLog.h"

bool MyModManager::LoadMod(const std::string& dl_path) {
    return _lib_mods.LoadMod(dl_path);
}

bool MyModManager::RegActor(const std::string& class_name, std::function<std::shared_ptr<MyActor>(const std::string&)> func) {
    if (_class_actors.find(class_name) != _class_actors.end()) {
        LOG(WARNING) << "reg " << class_name << " failed, " << " has exist";
        return false;
    }
    _class_actors[class_name] = func;
    return true;
}

bool MyModManager::RegWorker(const std::string& class_name, std::function<std::shared_ptr<MyWorker>(const std::string&)> func) {
    if (_class_workers.find(class_name) != _class_workers.end()) {
        LOG(WARNING) << "reg " << class_name << " failed, " << " has exist";
        return false;
    }
    _class_workers[class_name] = func;
    return true;
}

std::shared_ptr<MyActor> MyModManager::CreateActorInst(const std::string& mod_or_class_name, const std::string& actor_name) {
    if (_lib_mods.IsLoad(mod_or_class_name)) {
        return _lib_mods.CreateActorInst(mod_or_class_name, actor_name);
    }
    if (_class_actors.find(mod_or_class_name) != _class_actors.end()) {
        return _class_actors[mod_or_class_name](actor_name);
    }
    return nullptr;
}

std::shared_ptr<MyWorker> MyModManager::CreateWorkerInst(const std::string& mod_or_class_name, const std::string& worker_name) {
    if (_lib_mods.IsLoad(mod_or_class_name)) {
        return _lib_mods.CreateWorkerInst(mod_or_class_name, worker_name);
    }
    if (_class_workers.find(mod_or_class_name) != _class_workers.end()) {
        return _class_workers[mod_or_class_name](worker_name);
    }
    return nullptr;
}
