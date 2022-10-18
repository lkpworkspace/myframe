#pragma once
#include <sys/types.h>
#include <sys/socket.h>

#include <glog/logging.h>

#include "MyCUtils.h"
#include "MyCommon.h"

namespace myframe {

template<typename T>
class MyQueue final {
public:
    MyQueue() {
        CreateSockPair();
    }
    ~MyQueue() {
        CloseSockPair();
    }

    int GetFd0() { return _fd_pair[0]; }
    int GetFd1() { return _fd_pair[1]; }

    void Push(std::shared_ptr<T> data) {
        _data = data;
        char cmd_char = 'p';
        write(_fd_pair[0], &cmd_char, 1);
        read(_fd_pair[0], &cmd_char, 1);
    }

    std::shared_ptr<T> Pop() {
        std::shared_ptr<T> ret = nullptr;
        char cmd_char = '\0';
        read(_fd_pair[1], &cmd_char, 1);
        ret = _data;
        _data = nullptr;
        write(_fd_pair[1], &cmd_char, 1);
        return ret;
    }

private:
    bool CreateSockPair(){
        int res = -1;
        bool ret = true;

        res = socketpair(AF_UNIX, SOCK_DGRAM, 0, _fd_pair);
        if(res == -1) {
            LOG(ERROR) << "create sockpair failed";
            return false;
        }
        ret = my_set_nonblock(_fd_pair[0], false);
        if(!ret) {
            LOG(ERROR) << "set sockpair[0] block failed";
            return ret;
        }
        ret = my_set_nonblock(_fd_pair[1], false);
        if(!ret) {
            LOG(ERROR) << "set sockpair[1] block failed";
            return ret;
        }
        return ret;
    }
    void CloseSockPair() {
        if(-1 == close(_fd_pair[0])){
            LOG(ERROR) << "close sockpair[0]: " << my_get_error();
        }
        if(-1 == close(_fd_pair[1])){
            LOG(ERROR) << "close sockpair[1]: " << my_get_error();
        }
    }
    
    std::shared_ptr<T> _data;
    int _fd_pair[2] = {-1, -1};
};

} // namespace myframe