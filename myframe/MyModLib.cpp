#include <dlfcn.h>

#include "MyModLib.h"
#include "MyLog.h"
#include "MyModule.h"
#include "MyCUtils.h"

MyModLib::MyModLib() {
    pthread_rwlock_init(&_rw, NULL);
}

MyModLib::~MyModLib() {
    for (const auto& p : _mods) {
        UnloadMod(p.first);
    }
    pthread_rwlock_destroy(&_rw);
}

std::string MyModLib::GetModName(const std::string& full_path) {
    auto pos = full_path.find_last_of('/');
    pos = (pos == std::string::npos) ? -1 : pos;
    return full_path.substr(pos + 1);
}

bool MyModLib::LoadMod(const std::string& dlpath)
{
    auto dlname = GetModName(dlpath);
    pthread_rwlock_wrlock(&_rw);
    if(_mods.find(dlname) != _mods.end()){
        pthread_rwlock_unlock(&_rw);
        LOG(INFO) << "The " << dlname << " has loaded";
        return true;
    }

    void* dll_handle = dlopen(dlpath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if(dll_handle == nullptr) {
        pthread_rwlock_unlock(&_rw);
        LOG(ERROR) << "Open dll " << dlpath << " failed, " << (char*)dlerror();
        return false;
    }
    _mods[dlname] = dll_handle;
    pthread_rwlock_unlock(&_rw);
    return true;
}

bool MyModLib::IsLoad(const std::string& dlname)
{
    pthread_rwlock_rdlock(&_rw);
    auto res = _mods.find(dlname) != _mods.end();
    pthread_rwlock_unlock(&_rw);
    return res;
}

bool MyModLib::UnloadMod(const std::string& dlname)
{
    pthread_rwlock_wrlock(&_rw);
    if(_mods.find(dlname) == _mods.end()){
        pthread_rwlock_unlock(&_rw);
        return true;
    }

    if (dlclose(_mods[dlname])) {
        LOG(ERROR) << "lib close failed, " << (char*)dlerror();
    }
    _mods.erase(dlname);
    pthread_rwlock_unlock(&_rw);
    return true;
}

std::shared_ptr<MyModule> MyModLib::CreateModInst(const std::string& mod_name, const std::string& service_name) {
    pthread_rwlock_rdlock(&_rw);
    if(_mods.find(mod_name) == _mods.end()) {
        LOG(ERROR) << "Find " << mod_name << "." << service_name << " failed";
        return nullptr;
    }
    void* handle = _mods[mod_name];
    my_mod_create_func create = (my_mod_create_func)dlsym(handle, "my_mod_create");
    if(nullptr == create){
        pthread_rwlock_unlock(&_rw);
        LOG(ERROR) << "Load " << mod_name << "." << service_name << " module my_mod_create function failed";
        return nullptr;
    }
    auto mod = create(service_name);
    mod->_is_from_lib = true;
    mod->m_mod_name = mod_name;
    mod->m_service_name = service_name;
    pthread_rwlock_unlock(&_rw);
    return mod;
}
