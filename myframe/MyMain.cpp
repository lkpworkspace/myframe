#include "MyCommon.h"
#include "MyApp.h"
#include "MyLog.h"

int main(int argc, char** argv)
{
    // 利用构造函数初始化日志系统
    MyLog log;

    // 初始化并启动线程
    MyApp* app = MyApp::Create();
    if(false == app->ParseArg(argc, argv)){
        return -1;
    }

    // 开始事件循环
    return app->Exec();
}
