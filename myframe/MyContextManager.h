#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "MyCommon.h"
#include "MyList.h"

class MyContext;
class MyContextManager final
{
public:
    MyContextManager();
    virtual ~MyContextManager();

    /* 注册actor */
    bool RegContext(std::shared_ptr<MyContext> ctx);
  
    /* 获得actor名对应的actor */
    std::shared_ptr<MyContext> GetContext(const std::string& actor_name);
    
    /* 获得一个待处理的actor */
    std::shared_ptr<MyContext> GetContextWithMsg();

    /* 将有消息的actor放入链表 */
    void PushContext(std::shared_ptr<MyContext> ctx);

private:
    void PrintWaitQueue();
    
    /// 当前注册actor数量
    uint32_t _ctx_count;
    /// 待处理actor链表
    std::list<std::weak_ptr<MyContext>> _wait_queue;
    /// 读写锁
    pthread_rwlock_t _rw;
    /// key: context name
    /// value: context
    std::unordered_map<std::string, std::shared_ptr<MyContext>> _ctxs;

};
