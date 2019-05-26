#include <assert.h>

#include "MyCommon.h"
#include "MyFrame.h"
#include "MyApp.h"


int main(int argc, char** argv)
{
    argc = argc;
    argv = argv;

    // 初始化并启动线程
    MyApp app;

    // 加载服务模块
    assert(app.CreateContext("../CXXService/", "libtest_service.so", nullptr));
    assert(app.CreateContext("../CXXService/", "libtest_service2.so", nullptr));

    // 开始事件循环
    return app.Exec();
}
