#ifndef __MYMODULES_H__
#define __MYMODULES_H__
#include <unordered_map>

class MyModule;
class MyModules
{
public:
    MyModules();
    virtual ~MyModules();

    void SetModPath(const char* dl_path);

    bool LoadMod(const char* dlname);

    MyModule* CreateModInst(const char* mod_name);

    bool DestoryModInst(const char *mod_name, MyModule* mod);
private:
    std::string                            m_mod_path;
    std::unordered_map<std::string, void*> m_mods;
};

#endif
