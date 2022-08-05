#pragma once
#include <memory>
#include <functional>
#include <unordered_map>
#include "MyModule.h"
#include "MyModLib.h"

class MyModManager {
public:
    bool LoadMod(const std::string& dl_path);

    bool RegMod(const std::string& class_name, std::function<std::shared_ptr<MyModule>(const std::string&)> func);

    std::shared_ptr<MyModule> CreateModInst(const std::string& mod_or_class_name, const std::string& service_name);

private:
    MyModLib _lib_mods;
    std::unordered_map<std::string, std::function<std::shared_ptr<MyModule>(const std::string&)>> _class_mods;
};