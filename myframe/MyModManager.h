/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <functional>
#include <unordered_map>
#include "MyModLib.h"

namespace myframe {

class MyModManager final {
public:
    MyModManager();
    virtual ~MyModManager();

    bool LoadMod(const std::string& dl_path);

    bool RegActor(const std::string& class_name, std::function<std::shared_ptr<MyActor>(const std::string&)> func);

    bool RegWorker(const std::string& class_name, std::function<std::shared_ptr<MyWorker>(const std::string&)> func);

    std::shared_ptr<MyActor> CreateActorInst(const std::string& mod_or_class_name, const std::string& actor_name);

    std::shared_ptr<MyWorker> CreateWorkerInst(const std::string& mod_or_class_name, const std::string& worker_name);

private:
    MyModLib _lib_mods;
    std::unordered_map<std::string, std::function<std::shared_ptr<MyActor>(const std::string&)>> _class_actors;
    std::unordered_map<std::string, std::function<std::shared_ptr<MyWorker>(const std::string&)>> _class_workers;
    pthread_rwlock_t _class_actor_rw;
    pthread_rwlock_t _class_worker_rw;
};

} // namespace myframe