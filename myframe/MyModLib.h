#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <pthread.h>

class MyActor;
class MyWorker;
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
     * @brief 创建actor实例
     * 
     * @param mod_name eg: libtest.so
     * @param actor_name eg: /my/test
     * @return std::shared_ptr<MyActor> 
     */
    std::shared_ptr<MyActor> CreateActorInst(
        const std::string& mod_name,
        const std::string& actor_name);

    /**
     * @brief 创建Worker实例
     * 
     * @param mod_name eg: libtest.so
     * @param worker_name eg: /my/test
     * @return MyWorker*
     */
    MyWorker* CreateWorkerInst(
        const std::string& mod_name,
        const std::string& worker_name);

private:
    bool UnloadMod(const std::string& dlname);
    std::string GetModName(const std::string& full_path);

    std::unordered_map<std::string, void*> _mods;
    pthread_rwlock_t                       _rw;

};
