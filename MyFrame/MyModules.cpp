#include "MyModules.h"

#include <boost/log/trivial.hpp>

#include "MyModule.h"
#include "MyCUtils.h"

MyModules::MyModules() :
    m_mod_path("")
{
}

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
        BOOST_LOG_TRIVIAL(warning) << "The " << dlname << " has loaded";
        return true;
    }

    full_path = m_mod_path;
    full_path.append(dlname);

    inst = my_dll_open(full_path.c_str());
    if(inst == nullptr){
        BOOST_LOG_TRIVIAL(error) << "Load " << dlname << " module failed";
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
        BOOST_LOG_TRIVIAL(error) << "Load " << mod_name << " module my_mod_create function failed";
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
        BOOST_LOG_TRIVIAL(error) << "Load " << mod_name << " module my_mod_destory function failed";
        return false;
    }
    destory(mod);
    return true;
}
