#ifndef __MYMODULE_H__
#define __MYMODULE_H__
#include <stdint.h>
#include <stddef.h>

class MyContext;
class MyModule
{
public:
    MyModule(){}
    virtual ~MyModule(){}

    virtual int Init(MyContext* c, const char* param) = 0;
};

typedef MyModule* (*my_mod_create_func)(void);
typedef void (*my_mod_destory_func)(MyModule*);

#endif
