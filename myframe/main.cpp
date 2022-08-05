#include "gflags/gflags.h"

#include "MyCommon.h"
#include "MyApp.h"

int main(int argc, char** argv)
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 利用构造函数初始化日志系统
    MyLog log;

    // 初始化并启动线程
    MyApp* app = MyApp::Create();
    if(false == app->Init()){
        LOG(ERROR) << "Init failed";
        return -1;
    }

    // 开始事件循环
    return app->Exec();
}
