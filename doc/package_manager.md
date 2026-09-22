# package manager
使用包管理工具管理该项目

## 安装依赖
进入代码库根目录执行以下命令:
```sh
mypm install
```

## 构建项目
进入代码库根目录执行以下命令:
```sh
mypm run config
mypm run build
```

## 打包项目
进入代码库根目录执行以下命令:
```sh
mypm pack
```

## 插件工程
```sh
# 创建工程
mypm create myframe_plugin
# 进入工程目录
cd myframe_plugin
# 添加myframe依赖
mypm add myframe
# 构建
mypm run config
mypm run build
```