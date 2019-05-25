#include "MyModules.h"
#include "MyModule.h"
#include "MyCUtils.h"
#include "MyLog.h"

MyModules::MyModules() :
    m_mod_path("")
{}

MyModules::~MyModules()
{}

/* /home/ubuntu/ */
void MyModules::SetModPath(const char* dl_path)
{
    m_mod_path = dl_path;
}

/**
 * 0. 如果存在直接返回 true
 * 1. 加载动态库
 * 2. 存储到map中
 */
bool MyModules::LoadMod(const char* dlname)
{
    void* inst;
    std::string full_path;

    if(m_mods.find(dlname) != m_mods.end()){
        MYLOG(MYLL_WARN,("The %s has loaded\n", dlname));
        return true;
    }

    full_path = m_mod_path;
    full_path.append(dlname);

    inst = my_dll_open(full_path.c_str());
    if(inst == nullptr){
        MYLOG(MYLL_ERROR,("Load %s module failed\n", dlname));
        return false;
    }
    m_mods[dlname] = inst;
    return true;
}

MyModule* MyModules::CreateModInst(const char* mod_name)
{
    void* handle;
    if(m_mods.find(mod_name) == m_mods.end()) return nullptr;
    handle = m_mods[mod_name];
    my_mod_create_func create = (my_mod_create_func)my_dll_use(handle, "my_mod_create");
    if(nullptr == create){
        MYLOG(MYLL_ERROR,("Load %s module MyCreate function failed\n", mod_name));
        return nullptr;
    }
    return static_cast<MyModule*>(create());
}

bool MyModules::DestoryModInst(const char* mod_name, MyModule* mod)
{
    void* handle;
    if(m_mods.find(mod_name) == m_mods.end()){
        return false;
    }
    handle = m_mods[mod_name];
    my_mod_destory_func destory = (my_mod_destory_func)my_dll_use(handle, "my_mod_destory");
    if(nullptr == destory){
        MYLOG(MYLL_ERROR,("Load %s module MyDestory function failed\n", mod_name));
        return false;
    }
    destory(mod);
    return true;
}
