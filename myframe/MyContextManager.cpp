#include <string.h>
#include <assert.h>

#include "MyContextManager.h"
#include "MyContext.h"
#include "MyActor.h"
#include "MyLog.h"

namespace myframe {

MyContextManager::MyContextManager() 
    : _ctx_count(0)
{
    pthread_rwlock_init(&_rw, NULL);
}

MyContextManager::~MyContextManager()
{
    pthread_rwlock_destroy(&_rw);
}

bool MyContextManager::RegContext(std::shared_ptr<MyContext> ctx) {
    pthread_rwlock_wrlock(&_rw);
    if(_ctxs.find(ctx->GetModule()->GetActorName()) != _ctxs.end()){
        LOG(WARNING) << "reg the same actor name: " << ctx->GetModule()->GetActorName();
        pthread_rwlock_unlock(&_rw);
        return false;
    }
    LOG(INFO) << "reg actor " << ctx->GetModule()->GetActorName();
    _ctxs[ctx->GetModule()->GetActorName()] = ctx;
    pthread_rwlock_unlock(&_rw);
    return true;
}

std::shared_ptr<MyContext> MyContextManager::GetContext(const std::string& actor_name) {
    pthread_rwlock_rdlock(&_rw);
    if(_ctxs.find(actor_name) == _ctxs.end()){
        LOG(WARNING) << "not found " << actor_name;
        pthread_rwlock_unlock(&_rw);
        return nullptr;
    }
    auto ctx = _ctxs[actor_name];
    pthread_rwlock_unlock(&_rw);
    return ctx;
}

void MyContextManager::PrintWaitQueue() {
    DLOG(INFO) << "cur wait queue actor:";
    auto it = _wait_queue.begin();
    while (it != _wait_queue.end()) {
        DLOG(INFO) << "---> " << it->lock()->Print();
        ++it;
    }
}

std::shared_ptr<MyContext> MyContextManager::GetContextWithMsg() {
    if(_wait_queue.empty()) {
        return nullptr;
    }

    std::vector<std::shared_ptr<MyContext>> in_runing_context;
    std::shared_ptr<MyContext> ret = nullptr;
    while(!_wait_queue.empty()){
        if (_wait_queue.front().expired()) {
            _wait_queue.pop_front();
            continue;
        }
        auto ctx = _wait_queue.front().lock();
        if(ctx->IsRuning()){
            _wait_queue.pop_front();
            in_runing_context.push_back(ctx);
        }else{
            _wait_queue.pop_front();

            ctx->SetRuningFlag(true);
            ctx->SetWaitQueueFlag(false);
            ret = ctx;
            break;
        }
    }
    for (int i = 0; i < in_runing_context.size(); ++i) {
        DLOG(INFO) << in_runing_context[i]->GetModule()->GetActorName() << " is runing, move to wait queue back";
        _wait_queue.push_back(in_runing_context[i]);
    }
    return ret;
}

void MyContextManager::PushContext(std::shared_ptr<MyContext> ctx) {
    if(ctx->IsInWaitQueue()) {
        DLOG(INFO) << ctx->Print() << " already in wait queue, return";
        PrintWaitQueue();
        return;
    }
    ctx->SetWaitQueueFlag(true);
    _wait_queue.push_back(ctx);
    PrintWaitQueue();
}

} // namespace myframe