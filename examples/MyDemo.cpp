#include <iostream>
#include <string.h>

#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"


class MyDemo : public MyModule
{
public:
    MyDemo(){}
    virtual ~MyDemo(){}

    virtual int Init(MyContext* c, const char* param) override
    {
		uint32_t handle = my_handle(c);
        std::cout << "MyDemo init" << std::endl;
		my_callback(c, CB, nullptr);
		const char* hello = "hello,world";
        my_send(c, 0, handle, handle, 0, (void*)hello, strlen(hello));
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        std::string str((char*)msg,sz);
        std::cout << "----> from " << source << " to " << my_handle(context) << ": " << str << std::endl;
		return 0;
    }
};

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyDemo());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}