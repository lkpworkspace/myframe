#include "gflags/gflags.h"

#include "MyCommon.h"
#include "MyApp.h"
#include "MyFlags.h"

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 利用构造函数初始化日志系统
    myframe::MyLog log;

    // 初始化并启动线程
    auto app = std::make_shared<myframe::MyApp>();
    if(false == app->Init()) {
        LOG(ERROR) << "Init failed";
        return -1;
    }

    // 从配置文件加载服务
    if(!app->LoadModsFromConf(myframe::FLAGS_myframe_service_dir)) {
        LOG(ERROR) << "Load service failed, exit";
        return -1;
    }

    // 开始事件循环
    return app->Exec();
}
