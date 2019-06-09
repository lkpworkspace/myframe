#include <assert.h>

#include "MyCommon.h"
#include "MyFrame.h"
#include "MyApp.h"


int main(int argc, char** argv)
{
    argc = argc;
    argv = argv;

    // 初始化并启动线程
    MyApp* app = MyApp::Create(4);

    // 加载服务模块
    assert(app->CreateContext("../CXXService/", "libdemo.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_tcp_sock.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service2.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service3.so", nullptr));

    // 开始事件循环
    return app->Exec();
}
