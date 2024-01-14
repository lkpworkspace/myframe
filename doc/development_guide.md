# 开发手册
![myframe](/doc/pics/myframe_view.png)
## 术语介绍
- Actor：基础的执行单元
  - 驱动类型：消息驱动（被动执行）
  - 并发类型：单个Actor串行执行，多个Actor并行执行
  - 通信方式：接收消息，发送消息

- Worker：独立执行的线程，可以与框架单向通信
  - 驱动类型：自驱动（主动执行）
  - 并发类型：单个Worker串行执行，多个Worker并行执行
  - 通信方式：接收消息或者发送消息

- Service：由任意个Actor和Worker组成，通过描述文件展现; 它描述框架需要加载的库，需要创建的实例以及实例名称。
  - 例如下面这个例子:
  ```json
  {
    "type": "library",
    "lib": "Hello",
    "actor": {
      "HelloActor": [
        {
          "instance_name": "1",
          "instance_params": ""
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
  - 创建1个worker实例，名称是 worker.HelloReceiver.1
  - 创建1个worker实例，名称是 worker.HelloSender.1

- Module/Component：通常代指Actor或者Worker

## 开发
组件开发模式，通过编写组件开发业务。

### 创建组件工程
```sh
python path/to/myframe/bin/gen_mod_proj.py --dir="path/to/proj_dir/" --name="mod_name"
```

### 组件工程目录说明
- 源文件(template.cpp)
  - 提供actor/worker的使用模板,根据需求决定使用actor或者worker

- 配置文件：
  - Template.json：Service配置

### 组件工程构建安装
```sh
# 如果myframe库和依赖库安装到系统目录里，则不需要设置CMAKE_PREFIX_PATH变量
cmake -S . -B build -DCMAKE_PREFIX_PATH="path/to/myframe;path/to/jsoncpp;path/to/glog"
cmake --build build --config Release --target install
```

### 运行组件
```sh
/path/to/myframe/bin/launcher -p app ${组件名}.json
```

### 日志文件
```sh
path/to/myframe/log/app.INFO
```
