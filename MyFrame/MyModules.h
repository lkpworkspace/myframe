#ifndef __MYMODULES_H__
#define __MYMODULES_H__
#include <unordered_map>
#include <vector>
class MyModule;
class MyModules
{
public:
    MyModules();
    virtual ~MyModules();

    // 设置模块路径
    void SetModPath(const char* dl_path);
    // 加载模块动态库
    bool LoadMod(const char* dlname);
    // 创建模块实例
    MyModule* CreateModInst(const char* mod_name);
    // 销毁模块实例
    bool DestoryModInst(const char *mod_name, MyModule* mod);
private:
    std::string                            m_mod_path;
    std::unordered_map<std::string, void*> m_mods;
};

#endif
