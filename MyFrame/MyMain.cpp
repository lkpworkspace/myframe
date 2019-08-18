#include <assert.h>
#include "MyCommon.h"
#include "MyFrame.h"
#include "MyApp.h"

int main(int argc, char** argv)
{
    // 初始化并启动线程
    MyApp* app = MyApp::Create();
    if(false == app->ParseArg(argc, argv)){
        return -1;
    }
    // 加载服务模块
//    assert(app->CreateContext("../CXXService/", "liblandlord.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libgl_demo.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_timer.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libecho_tcp_server.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libdemo.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_tcp_server.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service2.so", nullptr));
//    assert(app->CreateContext("../CXXService/", "libtest_service3.so", nullptr));

    // 开始事件循环
    return app->Exec();
}
