#ifndef __MYMSG_H__
#define __MYMSG_H__
#include <stdint.h>

#include "MyCommon.h"
#include "MyList.h"

#define MY_MESSAGE_TYPE_MASK (SIZE_MAX >> 8)
#define MY_MESSAGE_TYPE_SHIFT ((sizeof(size_t)-1) * 8)

class MyMsg : public MyNode
{
public:
    MyMsg() :
        source(0),
        destination(0),
        session(0),
        data(nullptr),
        sz(0)
    {}
    virtual ~MyMsg(){}

    uint32_t  source;
    uint32_t  destination;
    int       session;
    void *    data;
    size_t    sz;

protected:

    /* 节点类型 */
    virtual enum ENUM_NODE_TYPE GetNodeType() override { return NODE_MSG; }
};

#endif // __MYEVENT_H__
