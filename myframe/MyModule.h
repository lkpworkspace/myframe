#ifndef __MYMODULE_H__
#define __MYMODULE_H__
#include <stdint.h>
#include <stddef.h>

#include <string>

class MyContext;
/**
 * MyModule - 服务模块
 * 
 * 所有编写的服务都需要继承该类，并重写Init()方法
 * 
 */
class MyModule
{
public:
    MyModule(){}
    MyModule(std::string mod_name, std::string service_name) : 
        m_mod_name(mod_name),
        m_service_name(service_name)
    {}
    virtual ~MyModule(){}

    /**
     * Init() - 服务初始化调用的初始化函数
     * @c:      服务指针
     * @param:  服务参数
     * 
     * @return: 未定义
     */
    virtual int Init(MyContext* c, const char* param) = 0;

    /* 服务使用的动态库名称 */
    std::string m_mod_name;
    /* 服务名称 */
    std::string m_service_name;
};

typedef MyModule* (*my_mod_create_func)(void);
typedef void (*my_mod_destory_func)(MyModule*);

#endif
