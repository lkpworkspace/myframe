#ifndef __MYCOMMON_H__
#define __MYCOMMON_H__
#include <string>
#include <vector>

#include <jsoncpp/json/json.h>

#include "MyLog.h"

class MyCommon {
public:
    static std::vector<std::string> GetDirFiles(const std::string& conf_path);
    static Json::Value LoadJsonFromFile(const std::string& json_file);
};

#endif // __MYCOMMON_H__
