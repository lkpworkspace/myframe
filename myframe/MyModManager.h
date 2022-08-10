#pragma once
#include <memory>
#include <functional>
#include <unordered_map>
#include "MyActor.h"
#include "MyModLib.h"

class MyModManager {
public:
    bool LoadMod(const std::string& dl_path);

    bool RegActor(const std::string& class_name, std::function<std::shared_ptr<MyActor>(const std::string&)> func);

    bool RegWorker(const std::string& class_name, std::function<MyWorker*(const std::string&)> func);

    std::shared_ptr<MyActor> CreateActorInst(const std::string& mod_or_class_name, const std::string& actor_name);

    MyWorker* CreateWorkerInst(const std::string& mod_or_class_name, const std::string& worker_name);

private:
    MyModLib _lib_mods;
    std::unordered_map<std::string, std::function<std::shared_ptr<MyActor>(const std::string&)>> _class_actors;
    std::unordered_map<std::string, std::function<MyWorker*(const std::string&)>> _class_workers;
};