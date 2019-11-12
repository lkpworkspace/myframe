#ifndef __MYMODULE_H__
#define __MYMODULE_H__
#include <stdint.h>
#include <stddef.h>

#include <string>

class MyContext;
class MyModule
{
public:
    MyModule(){}
    MyModule(std::string mod_name, std::string service_name) : 
        m_mod_name(mod_name),
        m_service_name(service_name)
    {}
    virtual ~MyModule(){}

    virtual int Init(MyContext* c, const char* param) = 0;

    std::string m_mod_name;
    std::string m_service_name;
};

typedef MyModule* (*my_mod_create_func)(void);
typedef void (*my_mod_destory_func)(MyModule*);

#endif
