#include "MyCommon.h"
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

std::vector<std::string> SplitMsgName(const std::string& name) {
    std::vector<std::string> name_list;
    std::string item;
    std::stringstream ss(name);
    while (std::getline(ss, item, '.')) {
        name_list.push_back(item);
    }
    return name_list;
}

std::vector<std::string> MyCommon::GetDirFiles(const std::string& conf_path) {
    std::vector<std::string> res;
    DIR* dir = opendir(conf_path.c_str());
    if (dir == nullptr) {
        return res;
    }
    struct dirent* entry = nullptr;
    while (nullptr != (entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            res.emplace_back(conf_path + entry->d_name);
        }
    }
    closedir(dir);
    return res;
}

Json::Value MyCommon::LoadJsonFromFile(const std::string& json_file) {
    std::ifstream ifs(json_file);
    if (!ifs.is_open()) {
        return Json::Value::null;
    }
    Json::Value root;
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(ifs, root)) {
        ifs.close();
        return Json::Value::null;
    }
    ifs.close();
    return root;
}