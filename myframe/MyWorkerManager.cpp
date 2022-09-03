
#include "MyWorkerManager.h"
#include "MyCommon.h"
#include "MyWorker.h"

MyWorkerManager::MyWorkerManager() {
    pthread_rwlock_init(&_rw, NULL);
}

MyWorkerManager::~MyWorkerManager() {
    pthread_rwlock_destroy(&_rw);
}

int MyWorkerManager::WorkerSize() {
    return _cur_worker_count;
}

std::shared_ptr<MyWorker> MyWorkerManager::Get(int handle) {
    pthread_rwlock_wrlock(&_rw);
    if(_workers.find(handle) == _workers.end()) {
        LOG(ERROR) << "can't find worker, handle " << handle;
        pthread_rwlock_unlock(&_rw);
        return nullptr;
    }
    auto ret = _workers[handle];
    pthread_rwlock_unlock(&_rw);
    return ret;
}

std::shared_ptr<MyWorker> MyWorkerManager::Get(const std::string& name) {
    pthread_rwlock_wrlock(&_rw);
    if(_name_handle_map.find(name) == _name_handle_map.end()) {
        LOG(ERROR) << "can't find worker, name " << name;
        pthread_rwlock_unlock(&_rw);
        return nullptr;
    }
    auto handle = _name_handle_map[name];
    auto ret = _workers[handle];
    pthread_rwlock_unlock(&_rw);
    return ret;
}

bool MyWorkerManager::Add(std::shared_ptr<MyWorker> worker) {
    int handle = worker->GetFd();
    pthread_rwlock_wrlock(&_rw);
    if(_workers.find(handle) != _workers.end()) {
        LOG(ERROR) << worker->GetWorkerName() << " reg handle " << handle << " has exist";
        pthread_rwlock_unlock(&_rw);
        return false;
    }
    _workers[handle] = worker;
    _name_handle_map[worker->GetWorkerName()] = handle;
    auto ev_type = worker->GetMyEventType();
    if (ev_type == MyEventType::WORKER_COMMON || ev_type == MyEventType::WORKER_USER) {
        ++_cur_worker_count;
    }
    pthread_rwlock_unlock(&_rw);
    return true;
}

void MyWorkerManager::Del(std::shared_ptr<MyWorker> worker) {
    int handle = worker->GetFd();
    pthread_rwlock_wrlock(&_rw);
    if(_workers.find(handle) == _workers.end()) {
        pthread_rwlock_unlock(&_rw);
        return;
    }
    _workers.erase(_workers.find(handle));
    _name_handle_map.erase(worker->GetWorkerName());
    auto ev_type = worker->GetMyEventType();
    if (ev_type == MyEventType::WORKER_COMMON || ev_type == MyEventType::WORKER_USER) {
        --_cur_worker_count;
    }
    pthread_rwlock_unlock(&_rw);
}

int MyWorkerManager::IdleWorkerSize() {
    return _idle_workers.size();
}

std::shared_ptr<MyWorker> MyWorkerManager::FrontIdleWorker() {
    if (_idle_workers.empty()) {
        return nullptr;
    } 
    return _idle_workers.front().lock();
}

void MyWorkerManager::PopFrontIdleWorker() {
   if (_idle_workers.empty()) {
        return;
    } 
    _idle_workers.pop_front();
}

void MyWorkerManager::PushBackIdleWorker(std::shared_ptr<MyWorker> worker) {
    _idle_workers.emplace_back(worker);
}

void MyWorkerManager::PushWaitWorker(std::shared_ptr<MyWorker>& worker) {
    worker->_state = MyWorkerState::WAIT;
}

void MyWorkerManager::WeakupWorker() {
    for (auto it = _weakup_workers.begin(); it != _weakup_workers.end();) {
        auto worker = it->lock();
        MyListAppend(worker->_que, worker->_recv);
        if (worker->_que.size() <= 0) {
            LOG(ERROR) << worker->GetWorkerName() << " has no msg, continue weak up";
        }
        it = _weakup_workers.erase(it);
        worker->_state == MyWorkerState::RUN;
        worker->SendCmdToWorker(MyWorkerCmd::RUN);
    }
}

void MyWorkerManager::DispatchWorkerMsg(std::shared_ptr<MyMsg>& msg) {
    std::string worker_name = msg->GetDst();
    if (_name_handle_map.find(worker_name) == _name_handle_map.end()) {
        LOG(ERROR) << "can't find worker " << worker_name 
            << ", drop msg: from " << msg->GetSrc() << " to "
            << msg->GetDst();
        return;
    }
    auto worker = Get(worker_name);
    auto worker_type = worker->GetMyEventType();
    if (worker_type == MyEventType::WORKER_TIMER
        || worker_type == MyEventType::WORKER_COMMON) {
        LOG(WARNING) << worker_name << " unsupport recv msg, drop it";
        return;
    }
    worker->_recv.emplace_back(msg);
    if (worker->_state == MyWorkerState::WAIT) {
        worker->_state == MyWorkerState::WEAKUP;
        _weakup_workers.emplace_back(worker);
    }
}
