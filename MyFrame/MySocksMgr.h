#ifndef __MYSOCKSMGR_H__
#define __MYSOCKSMGR_H__
#include <string>
#include <unordered_map>
#include <pthread.h>

struct requ_send {
    int id;
};

struct my_requ_pkg{
    uint8_t   head;
    union{
       char buffer[256];
       struct requ_send send;
    }u;
};


class MySock;
class MyContext;
class MySocksMgr
{
public:
    MySocksMgr();
    virtual ~MySocksMgr();

    /* reg and unreg */
    uint32_t RegId(MySock* sock);
    void UnregId(MySock* sock);
    void UnregId(uint32_t id);
    MySock* GetSock(uint32_t id);

    /* return id */
    // for server socket
    int Listen(MyContext* ctx, const char* addr, int port, int backlog);
    // for client socket
    int Connect(MyContext* ctx, const char* addr, int port);
    // bind socket
    int Bind(MyContext* ctx, int fd);
    // Accept
    MySock *Accept(MySock* sock);
    // Close
    void ForceClose(uint32_t id);
private:
    MySock* NewSock(uint32_t handle, int fd, int protocol);
    int DoBind(const char* host, int port, int portocol, int* family);
    int DoListen(const char* host, int port, int backlog);
    std::unordered_map<uint32_t,MySock*> m_ids;
    pthread_rwlock_t                     m_rw;
    uint32_t                             m_alloc_id;
};


#endif
