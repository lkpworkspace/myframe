#ifndef __MYMODULES_H__
#define __MYMODULES_H__
#include <unordered_map>
#include <vector>
#include <pthread.h>

class MyModule;
class MyModules
{
public:
    MyModules();
    virtual ~MyModules();

    // 设置模块路径
    void SetModPath(const char* dl_path);
    // 是否已经加载动态库
    bool IsLoad(const char* dlname);
    // 加载模块动态库
    bool LoadMod(const char* dlname);
    // 卸载模块动态库
    bool UnloadMod(const char* dlname);
    
    // 创建模块实例
    MyModule* CreateModInst(const char* mod_name, const char* service_name);
    // 销毁模块实例
    bool DestoryModInst(const char *mod_name, MyModule* mod);
private:
    std::string                            m_mod_path;
    std::unordered_map<std::string, void*> m_mods;
    pthread_rwlock_t                       m_rw;
};

#endif
