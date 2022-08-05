#include "MyModManager.h"
#include <jsoncpp/json/json.h>
#include "MyLog.h"

bool MyModManager::LoadMod(const std::string& dl_path) {
    return _lib_mods.LoadMod(dl_path);
}

bool MyModManager::RegMod(const std::string& class_name, std::function<std::shared_ptr<MyModule>(const std::string&)> func) {
    if (_class_mods.find(class_name) != _class_mods.end()) {
        LOG(WARNING) << "reg " << class_name << " failed, " << " has exist";
        return false;
    }
    _class_mods[class_name] = func;
    return true;
}

std::shared_ptr<MyModule> MyModManager::CreateModInst(const std::string& mod_or_class_name, const std::string& service_name) {
    if (_lib_mods.IsLoad(mod_or_class_name)) {
        return _lib_mods.CreateModInst(mod_or_class_name, service_name);
    }
    if (_class_mods.find(mod_or_class_name) != _class_mods.end()) {
        return _class_mods[mod_or_class_name](service_name);
    }
}