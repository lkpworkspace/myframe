#ifndef __MYCOMMON_H__
#define __MYCOMMON_H__
#include <memory>
#include <string>
#include <vector>
#include <list>

#include <jsoncpp/json/json.h>

#include "MyLog.h"

template<typename T>
void MyListAppend(std::list<std::shared_ptr<T>>& src, std::list<std::shared_ptr<T>>& dst) {
    src.insert(src.end(), dst.begin(), dst.end());
    dst.clear();
}

class MyCommon {
public:
    static std::vector<std::string> GetDirFiles(const std::string& conf_path);
    static Json::Value LoadJsonFromFile(const std::string& json_file);
};

#endif // __MYCOMMON_H__
