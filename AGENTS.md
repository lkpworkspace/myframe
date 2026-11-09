# myframe 协作指南

本文面向在本仓库工作的贡献者和编码代理。myframe 是一个 C++17 组件化框架：程序由消息驱动的 Actor 和自主运行的 Worker 组成；组件既可在进程内注册，也可按 JSON 服务描述从动态库加载。

## 仓库结构

| 路径 | 用途 |
| --- | --- |
| myframe/ | 核心库及其公共头文件。app.* 负责启动、分发和退出；上下文、邮箱、事件、轮询器与管理器共同实现运行时。 |
| myframe/platform/ | 由 platform.h 选择的平台实现。Linux 使用 epoll 与 socket pair；其他已支持目标使用通用轮询器/通道实现。 |
| launcher/ | myframe_launcher：读取配置、初始化日志与 App、加载服务并进入事件循环。 |
| examples/ | 动态库组件及相应服务 JSON；新增插件时优先参照这里的范例。 |
| tests/ | 覆盖 API、调度、消息传递及吞吐量的独立 CTest 可执行程序。 |
| python/ | 可选的 SWIG/Python 绑定与 Python 示例。 |
| 3rd/ | 为本地预设构建第三方依赖的 CMake 工程。 |
| cmake/ | CMake 包安装辅助逻辑。 |
| doc/ | 中文开发、包管理和发布文档；README_en.md 是英文概览。 |
| tools/ | 随安装产物提供的环境设置脚本。 |

myframe/ 中的 config.h（来自 config.h.in）和 export.h（由 CMake 的 GenerateExportHeader 生成）属于生成文件。不要直接修改它们；应修改模板或 CMake 配置。

## 架构与运行时约束

### 生命周期

App::Init() 会创建轮询器、事件连接管理器、通用 Worker 线程池和定时器 Worker。随后可通过 LoadServiceFrom*() 加载服务，或通过 ModManager 注册组件并调用 AddActor() / AddWorker() 添加实例。App::Exec() 运行事件循环，使用 App::Quit() 正常终止。

Actor 的初始化入口为 Actor::Init()，消息处理入口为 Actor::Proc()。单个 Actor 实例串行处理消息，不同实例可并行执行。Actor 地址固定为 actor.<类型名>.<实例名>。

Worker 拥有独立线程，生命周期依次为 Init()、重复调用 Run()、Exit()。Run() 必须定期返回，或在收到 Stop() 请求时能够解除阻塞。Worker 地址为 worker.<类型名>.<实例名>。使用 DispatchMsg() 非阻塞地处理框架消息；需要等待框架工作时使用 DispatchAndWaitMsg()。

不要将 Actor::GetApp() 或 Worker::GetApp() 返回的 std::shared_ptr<App> 保存到成员变量或静态变量中；公共 API 明确说明这会破坏正常退出流程。

### 消息机制

- 普通消息经由 Mailbox 收发；按需要在发送前设置目标、名称、载荷和传输模式。
- Actor::Publish() 与 Subscribe() 建立发布/订阅路由。主题由发布者/订阅目标地址及可选消息名构成。
- Actor::Request() 会生成请求 ID，并通过回调在内部消费相应响应；服务端应使用 Actor::Response() 回复，框架会补齐响应的路由和请求元数据。
- Actor::Timeout(name, ticks) 在 ticks * 10 ms 后投递定时器消息。定时器为一次性；周期任务应在 Proc() 中重新设置下一次定时器。
- myframe/macros.h 定义了订阅、发布、定时、请求和响应等保留消息类型。修改分发逻辑时必须保持这些语义。

队列默认值由 Arguments 或启动器配置提供。Actor 的 instance_config.pending_queue_size 和 instance_config.run_queue_size 可覆盖默认值；-1 表示不限制，默认运行队列大小为 2。

### 组件加载

服务描述包含 type（"library" 或 "class"）以及可选的 actor / worker 映射。动态库服务还必须指定 lib；Common::GetLibName() 支持将简写名称映射为平台库名。每个组件实例都需要 instance_name，并可带有 instance_config。

动态组件库必须以 C 链接方式导出以下精确的工厂符号，并使用导出宏：

~~~cpp
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name);

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name);
~~~

仅为支持的类型名返回实例，其他名称返回 nullptr。参考 examples/example_actor_helloworld.cpp 和 examples/example_worker_io.cpp。对于进程内组件，先通过 ModManager::RegActor 或 RegWorker 注册工厂函数，再以模块名 "class" 创建实例。

## 构建、测试与风格

依赖项为 jsoncpp 和 glog；部分平台上 glog 还需要 gflags。启用 MYFRAME_ENABLE_PYBIND=ON 时还需要 SWIG 与 Python 开发头文件。

文档中的本地工作流会将依赖构建到 .mypm/，安装项目后执行 CTest：

~~~sh
cmake -S 3rd -B .mypm/build_3rd -DCMAKE_INSTALL_PREFIX=.mypm/install \
  -DMYFRAME_ENABLE_PYBIND=ON
cmake --build .mypm/build_3rd --config Release
cmake --preset ReleaseLocal
cmake --build --preset ReleaseLocal
ctest --preset ReleaseLocal
~~~

DebugLocal 与 AsanLocal 分别用于调试与 AddressSanitizer 检查。非 Local 的预设使用 .mypm/deps 下的依赖；包管理方式为先执行 mypm install，再执行 mypm run build。CTest 命令指向安装前缀，因此运行 CTest 前必须构建 install 目标（提供的 build preset 已包含该目标）。

提交 C++ 改动前，执行与改动相关的测试程序或 CTest 预设，并运行：

~~~sh
bash cpplint.bash
~~~

格式遵循 .clang-format：基于 Google 的 C++ 风格、两个空格缩进、80 列宽度、自动排序及分组 include。Lint 脚本会跳过第三方、生成和构建产物。CI 覆盖 Linux（C++17/C++20，Debug/Release）、macOS（C++17/C++20）及 Windows（C++17）；除非在 platform.h 中正确隔离并在相应平台验证，否则不要加入仅限单一平台的改动。

## 改动指引

- 保持 myframe/ 中公共头文件与实现同步。若新公共符号会跨动态库 ABI 使用，需添加 MYFRAME_EXPORT。
- 不要破坏生命周期与线程边界。Actor 状态可依赖 Proc() 的串行调用；Worker 和管理器状态必须显式同步。
- 修改插件约定时同步更新相应 JSON 服务样例。组件的 CMake target 与描述文件的 lib 名称在平台库名归一化后必须一致。
- 修改核心行为时，在 tests/ 添加或更新聚焦的测试程序，并在 tests/CMakeLists.txt 注册。将耗时吞吐测试与轻量正确性测试分开。
- 安装方式、公共 API、配置、服务结构或发布行为变化时，更新 README.md、README_en.md 或 doc/。对重叠内容保持中文主文档与英文概览一致。
- 调整发布版本时，同时更新 CMakeLists.txt 中的 MYFRAME_VERSION 与 CMakePresets.json 中的对应值，这是发布指南要求。

## 快速定位

- 追踪公共行为时，从 myframe/app.h、actor.h、worker.h、msg.h 和 mailbox.h 开始。
- 服务描述校验与动态加载逻辑位于 myframe/mod_manager.cpp。
- 调整调度、消息分发、响应回调或退出流程时，检查 myframe/actor_context.cpp、worker_context.cpp 和 app.cpp。
- 运行配置和命令行启动行为位于 launcher/launcher.cpp 与 launcher/conf/myframe.json。
