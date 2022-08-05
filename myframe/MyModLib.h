#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <pthread.h>

class MyModule;
class MyModLib final
{
public:
    MyModLib();
    ~MyModLib();

    /**
     * @brief 是否加载动态库
     * 
     * @param dlname lib name
     * @return true 
     * @return false 
     */
    bool IsLoad(const std::string& dlname);

    /**
     * @brief 加载模块动态库
     * 
     * @param dlname full lib path
     * @return true 
     * @return false 
     */
    bool LoadMod(const std::string& dlpath);
    
    /**
     * @brief 创建模块实例
     * 
     * @param mod_name eg: libtest.so
     * @param service_name eg: /my/test
     * @return std::shared_ptr<MyModule> 
     */
    std::shared_ptr<MyModule> CreateModInst(
        const std::string& mod_name,
        const std::string& service_name);

private:
    bool UnloadMod(const std::string& dlname);
    std::string GetModName(const std::string& full_path);

    std::unordered_map<std::string, void*> _mods;
    pthread_rwlock_t                       _rw;

};
