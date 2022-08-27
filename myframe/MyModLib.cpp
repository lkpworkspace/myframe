#include <dlfcn.h>

#include "MyModLib.h"
#include "MyCUtils.h"
#include "MyLog.h"
#include "MyActor.h"
#include "MyWorker.h"

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
        DLOG(INFO) << dlname << " has loaded";
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
    LOG(INFO) << "Load lib " << dlpath;
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

std::shared_ptr<MyWorker> MyModLib::CreateWorkerInst(
    const std::string& mod_name,
    const std::string& worker_name) {
    pthread_rwlock_rdlock(&_rw);
    if(_mods.find(mod_name) == _mods.end()) {
        LOG(ERROR) << "Find " << mod_name << "." << worker_name << " failed";
        return nullptr;
    }
    void* handle = _mods[mod_name];
    my_worker_create_func create = (my_worker_create_func)dlsym(handle, "my_worker_create");
    if(nullptr == create){
        pthread_rwlock_unlock(&_rw);
        LOG(ERROR) << "Load " << mod_name << "." << worker_name << " module my_worker_create function failed";
        return nullptr;
    }
    auto worker = create(worker_name);
    pthread_rwlock_unlock(&_rw);
    return worker;
}

std::shared_ptr<MyActor> MyModLib::CreateActorInst(const std::string& mod_name, const std::string& actor_name) {
    pthread_rwlock_rdlock(&_rw);
    if(_mods.find(mod_name) == _mods.end()) {
        LOG(ERROR) << "Find " << mod_name << "." << actor_name << " failed";
        return nullptr;
    }
    void* handle = _mods[mod_name];
    my_actor_create_func create = (my_actor_create_func)dlsym(handle, "my_actor_create");
    if(nullptr == create){
        pthread_rwlock_unlock(&_rw);
        LOG(ERROR) << "Load " << mod_name << "." << actor_name << " module my_actor_create function failed";
        return nullptr;
    }
    auto mod = create(actor_name);
    mod->_is_from_lib = true;
    mod->m_mod_name = mod_name;
    mod->m_actor_name = actor_name;
    pthread_rwlock_unlock(&_rw);
    return mod;
}
