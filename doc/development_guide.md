# 开发手册
![myframe](/doc/pics/myframe_view.png)

## 安装和构建
- 方法一: 通过包管理的方式构建
  - [安装和构建说明](package_manager.md)
- 方法二: 直接通过cmake脚本构建
  - [查看README](../README.md)

## 目录结构
```txt
产出目录
|- bin
|- conf
|- include
|- lib
|- log
 - service
```
- bin
  - 主要存放启动器(myframe_launcher),环境设置脚本(myframe_setup)等可执行程序
- conf
  - 放置启动器配置文件以及其它用户配置
- incllude
  - 开发头文件
- lib
  - 所有库文件
- log
  - 程序生成日志目录
- service
  - 插件描述文件存放目录

## 术语介绍
- Actor：基础的执行单元
  - 驱动类型：消息驱动（被动执行）
  - 并发类型：单个Actor串行执行，多个Actor并行执行
  - 通信方式：接收消息，发送消息

- Worker：独立执行的线程，可以与框架单向通信
  - 驱动类型：自驱动（主动执行）
  - 并发类型：单个Worker串行执行，多个Worker并行执行
  - 通信方式：接收消息或者发送消息

- Service：由任意个Actor和Worker组成，通过描述文件展现; 详见[描述文件](#组件描述文件)

- Module/Component：通常代指Actor或者Worker

## 组件系统
组件主要由两部分构成:
- 组件描述文件
- 组件动态库

### 组件描述文件
- 该描述文件通常存放到service目录
```json
{
  "type": "library",
  "lib": "Hello",
  "actor": {
    "HelloActor": [
      {
        "instance_name": "1",
        "instance_config": {
          "pending_queue_size":-1,
          "run_queue_size":2
        }
      }
    ]
  },
  "worker": {
    "HelloReceiver": [
      {
        "instance_name": "1"
      }
    ],
    "HelloSender": [
      {
        "instance_name": "1"
      }
    ]
  }
}
```
- "type":"library": 服务通过库的形式提供
- "lib":"Hello": 需要加载的库名称
  - 可以写简略库名，比如 Hello
  - 也可以写库的全名，比如libHello.so, Hello,dll
- 创建1个actor实例，名称是 actor.HelloActor.1
  - pending_queue_size是这个等待队列长度,-1是无限制
  - run_queue_size是设置每次执行消费最大消息数量, -1是无限制
- 创建1个worker实例，名称是 worker.HelloReceiver.1
- 创建1个worker实例，名称是 worker.HelloSender.1

### 组件动态库
- 该动态库通常存放在lib目录下

### 组件加载

#### 通过myframe_launcher加载
- 更详细的用法可以通过 myframe_launcher -h 查看
```sh
# 进入bin目录
source myframe_setup.sh
./myframe_launcher -p app ${组件名}.json
```
